#pragma once

#include <string>
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

static void ReadJsonFile(const std::string& path, rapidjson::Document& out_document)
{
	FILE* file_pointer = nullptr;
	file_pointer = fopen(path.c_str(), "rb");
	char buffer[65536];
	rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
	out_document.ParseStream(stream);
	std::fclose(file_pointer);

	if (out_document.HasParseError()) {
		rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
		std::cout << "error parsing json at [" << path << "]" << std::endl;
		exit(0);
	}
}