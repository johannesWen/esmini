/* 
 * esmini - Environment Simulator Minimalistic 
 * https://github.com/esmini/esmini
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * 
 * Copyright (c) partners of Simulation Scenarios
 * https://sites.google.com/view/simulationscenarios
 */

#include "Replay.hpp"
#include "ScenarioGateway.hpp"
#include "CommonMini.hpp"

using namespace scenarioengine;


Replay::Replay(std::string filename) : time_(0.0), index_(0), repeat_(false)
{
	file_.open(filename, std::ofstream::binary);
	if (file_.fail())
	{
		LOG("Cannot open file: %s", filename.c_str());
		throw std::invalid_argument(std::string("Cannot open file: ") + filename);
	}

	file_.read((char*)&header_, sizeof(header_));
	LOG("Recording %s opened. odr: %s model: %s", filename.c_str(), header_.odr_filename, header_.model_filename);

	while (!file_.eof())
	{
		ObjectStateStruct data;

		file_.read((char*)&data, sizeof(data));

		if (!file_.eof())
		{
			data_.push_back(data);
		}
	}

	if (data_.size() > 0)
	{
		// Register first entry timestamp as starting time
		time_ = data_[0].timeStamp;
		startTime_ = time_;
		startIndex_ = 0;

		// Register last entry timestamp as stop time
		stopTime_ = data_[data_.size() - 1].timeStamp;
		stopIndex_ = FindIndexAtTimestamp(stopTime_);
	}
}

Replay::~Replay()
{
	data_.clear();
}


void Replay::GoToTime(double timestamp)
{
	if (timestamp > stopTime_)
	{
		if (repeat_)
		{
			index_ = startIndex_;
			time_ = startTime_;
		}
		else
		{
			index_ = stopIndex_;
			time_ = stopTime_;
		}
	}
	else
	{
		index_ = FindIndexAtTimestamp(timestamp, index_);
		time_ = timestamp;
	}
}

void Replay::GoToNextFrame()
{
	double ctime = data_[index_].timeStamp;
	for (size_t i = index_+1; i < data_.size(); i++)
	{
		if (data_[i].timeStamp > ctime)
		{
			GoToTime(data_[i].timeStamp);
			break;
		}
	}
}

void Replay::GoToPreviousFrame()
{
	if (index_ > 0)
	{
		GoToTime(data_[index_ -1].timeStamp);
	}
}

int Replay::FindIndexAtTimestamp(double timestamp, int startSearchIndex)
{
	size_t i = 0;

	if (timestamp < time_)
	{
		// start search from beginning
		startSearchIndex = 0;
	}

	for (i = startSearchIndex; i < data_.size(); i++)
	{
		if (data_[i].timeStamp >= timestamp)
		{
			break;
		}
	}

	return (int)(MIN(i, data_.size()-1));
}

ObjectStateStruct* Replay::GetState(int id)
{
	// Read all vehicles at current timestamp
	if (index_ + id > data_.size() - 1 || data_[index_ + id].id != id)
	{
		return 0;
	}
	return &data_[index_ + id];
}

void Replay::SetStartTime(double time)
{ 
	startTime_ = time;
	if (time_ < startTime_)
	{
		time_ = startTime_;
	}

	startIndex_ = FindIndexAtTimestamp(startTime_);
}

void Replay::SetStopTime(double time) 
{ 
	stopTime_ = time; 
	if (time_ > stopTime_)
	{
		time_ = stopTime_;
	}

	stopIndex_ = FindIndexAtTimestamp(stopTime_);
}
