/**
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

#ifndef __EVENT_WRITE_OPERATION_H_
#define __EVENT_WRITE_OPERATION_H_

#include <cstdint>

namespace opendnp3
{

class ObjectWriter;
class SelectionCriteria;

typedef bool(*EventHeaderWriteFunc)(ObjectWriter& writer, SelectionCriteria& criteria, uint32_t maxObjects);

class EventWriteOperation
{
	public:

	EventWriteOperation();

	EventWriteOperation(EventHeaderWriteFunc pWriter_, uint32_t count_);

	bool Invoke(ObjectWriter& writer, SelectionCriteria& criteria);

	bool IsDefined() const;
	
	void Clear();	

	EventHeaderWriteFunc pWriter;
	uint32_t count;
};

}

#endif
