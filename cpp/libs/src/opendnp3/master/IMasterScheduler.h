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
#ifndef OPENDNP3_IMASTERSCHEDULER_H
#define OPENDNP3_IMASTERSCHEDULER_H

#include "opendnp3/master/IMasterTask.h"

namespace opendnp3
{

/**
* Interface used by master contexts to schedule tasks
*/
class IMasterScheduler
{

public:

	/**
	* Add a single task to the scheduler. The tasks will be started asynchronously,
	* i.e. not by the call to this method
	*/
	virtual void Add(const std::shared_ptr<IMasterTask>& task, IMasterTaskRunner& runner) = 0;

	/**
	* Complete the task the currently running task. Reschedule only if it is as recurring task
	*/
	virtual void RemoveTasksFor(const IMasterTaskRunner& runner) = 0;

	/**
	* Remove all tasks associated with this context
	*/
	virtual bool CompleteCurrentFor(const IMasterTaskRunner& runner, bool reschedule) = 0;

	/**
	* Add multiple tasks in one call
	*/
	void Add(std::initializer_list<std::shared_ptr<IMasterTask>> tasks, IMasterTaskRunner& runner)
	{
		for (auto& task : tasks) this->Add(task, runner);
	}

};

}

#endif
