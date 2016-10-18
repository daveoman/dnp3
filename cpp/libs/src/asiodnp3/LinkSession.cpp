/*
* Licensed to Green Energy Corp (www.greenenergycorp.com) under one or
* more contributor license agreements. See the NOTICE file distributed
* with this work for additional information regarding copyright ownership.
* Green Energy Corp licenses this file to you under the Apache License,
* Version 2.0 (the "License"); you may not use this file except in
* compliance with the License.  You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* This project was forked on 01/01/2013 by Automatak, LLC and modifications
* may have been made to this file. Automatak, LLC licenses these modifications
* to you under the terms of the License.
*/

#include "asiodnp3/LinkSession.h"

#include <openpal/logging/LogMacros.h>

#include <opendnp3/LogLevels.h>

using namespace openpal;
using namespace asiopal;
using namespace opendnp3;

namespace asiodnp3
{

LinkSession::LinkSession(
    const openpal::Logger& logger,
    uint64_t sessionid,
    const std::shared_ptr<IResourceManager>& manager,
    const std::shared_ptr<IListenCallbacks>& callbacks,
	const std::shared_ptr<asiopal::Executor>& executor,
    const std::shared_ptr<asiopal::IAsyncChannel>& channel) :
	logger(logger),
	session_id(sessionid),
	manager(manager),
	callbacks(callbacks),
	executor(executor),
	parser(logger, &stats),
	first_frame_timer(*executor),
	channel(channel)
{

}

void LinkSession::Shutdown()
{
	auto shutdown = [self = shared_from_this()]()
	{
		self->first_frame_timer.Cancel();
		self->channel->Shutdown();
	};
	this->executor->Post(shutdown);
}

void LinkSession::SetLogFilters(openpal::LogFilters filters)
{
	this->logger.SetFilters(filters);
}

void LinkSession::BeginTransmit(const openpal::RSlice& buffer, opendnp3::ILinkSession& session)
{
	auto callback = [self = shared_from_this(), buffer, &session](const std::error_code & ec)
	{
		if (ec)
		{
			// we'll let the failed read close the session
			SIMPLE_LOG_BLOCK(self->logger, flags::WARN, ec.message().c_str());
			session.OnTransmitResult(false);
		}
		else
		{
			session.OnTransmitResult(true);
		}
	};

	this->channel->BeginWrite(buffer, callback);
}

bool LinkSession::OnFrame(const LinkHeaderFields& header, const openpal::RSlice& userdata)
{
	if (this->stack)
	{
		this->stack->OnFrame(header, userdata);
	}
	else
	{
		this->first_frame_timer.Cancel();

		this->callbacks->OnFirstFrame(this->session_id, header, *this);

		if (this->stack)
		{
			this->stack->OnLowerLayerUp();

			// push the frame into the newly created stack
			this->stack->OnFrame(header, userdata);
		}
		else
		{
			SIMPLE_LOG_BLOCK(this->logger, flags::WARN, "No master created. Closing socket.");
			this->channel->Shutdown();
		}
	}

	return true;
}

std::shared_ptr<IMasterSession> LinkSession::AcceptSession(
    const std::string& loggerid,
    std::shared_ptr<opendnp3::ISOEHandler> SOEHandler,
    std::shared_ptr<opendnp3::IMasterApplication> application,
    const MasterStackConfig& config)
{
	if (this->stack)
	{
		SIMPLE_LOG_BLOCK(this->logger, flags::ERR, "SocketSession already has a master bound");
		return nullptr;
	}

	// rename the logger id to something meaningful
	this->logger.Rename(loggerid.c_str());

	this->stack = MasterSessionStack::Create(
	                  this->logger,
	                  this->executor,
	                  SOEHandler,
	                  application,
	                  shared_from_this(),
	                  *this,
	                  config
	              );

	return stack;
}

void LinkSession::Start()
{	
	auto timeout = [self = shared_from_this()]()
	{
		SIMPLE_LOG_BLOCK(self->logger, flags::ERR, "Timed out before receving a frame. Closing socket.");
		self->channel->Shutdown();
	};

	this->first_frame_timer.Start(this->callbacks->GetFirstFrameTimeout(), timeout);

	this->BeginReceive();
}

void LinkSession::BeginReceive()
{
	auto self(shared_from_this());
	auto callback = [self](const std::error_code & ec, std::size_t num)
	{
		if (ec)
		{
			SIMPLE_LOG_BLOCK(self->logger, flags::WARN, ec.message().c_str());

			// if we created a master stack, tell it to shutdown
			if (self->stack)
			{
				self->stack->OnLowerLayerDown();
			}

			self->callbacks->OnConnectionClose(self->session_id, self->stack);

			// run any shutdown actions
			self->manager->Detach(self);

			// release our reference to the stack
			self->stack.reset();
		}
		else
		{
			self->parser.OnRead(static_cast<uint32_t>(num), *self);
			self->BeginReceive();
		}
	};

	auto dest = parser.WriteBuff();
	channel->BeginRead(dest, callback);
}

}
