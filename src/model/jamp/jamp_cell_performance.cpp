#include "jamp_cell_performance.h"


int32 JampCellPerformance::Deserialize(std::string jsonSrc)
{
	Reader reader;
	Value value;

	reader.parse(jsonSrc, value, false);

	return DeserializeFromValue(value);
}

int32 JampCellPerformance::DeserializeFromValue(Value src){

	if (!src["cellId"].empty())
		cellId = src["cellId"].asString();

	if (!src["classification"].empty()){
		std::string classificationStr = src["classification"].asString();
		if (classificationStr.compare("PERFECT") == 0)
			classification = PERFECT;
		else if (classificationStr.compare("GOOD") == 0)
			classification = GOOD;
		else if (classificationStr.compare("BAD") == 0)
			classification = BAD;
		else if (classificationStr.compare("MISS") == 0)
			classification = MISS;
	}

	if (!src["notesPerformance"].empty()){
		notesPerformance.DeserializeFromValue(src["notesPerformance"]);
	}

	if (!src["timeStamp"].empty())
		timeStamp = src["timeStamp"].asUInt64();

	return 0;
}


std::string JampCellPerformance::Serialize(){
	FastWriter writer;
	return writer.write(SerializeToValue());
}

Value JampCellPerformance::SerializeToValue(){
	Value root;

	root["cellId"] = cellId;

	switch (classification){
	case PERFECT:
		root["classification"] = "PERFECT";
		break;
	case GOOD:
		root["classification"] = "GOOD";
		break;
	case BAD:
		root["classification"] = "BAD";
		break;
	case MISS:
		root["classification"] = "MISS";
		break;
	}

	root["notesPerformance"] = notesPerformance.SerializeToValue();

	root["timeStamp"] = timeStamp;

	return root;
}