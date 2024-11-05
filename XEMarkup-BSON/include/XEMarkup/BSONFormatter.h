/*========================================================

 XEMarkup - BSON Formatter
 Copyright (C) 2024 Jon Bogert (jonbogert@gmail.com)

 This software is provided 'as-is', without any express or implied warranty.
 In no event will the authors be held liable for any damages arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it freely,
 subject to the following restrictions:

 1. The origin of this software must not be misrepresented;
	you must not claim that you wrote the original software.
	If you use this software in a product, an acknowledgment
	in the product documentation would be appreciated but is not required.

 2. Altered source versions must be plainly marked as such,
	and must not be misrepresented as being the original software.

 3. This notice may not be removed or altered from any source distribution.

========================================================*/

#ifndef XE_BSONFORMATTER_H
#define XE_BSONFORMATTER_H

#include <XEMarkup/IFormatter.h>

namespace xe
{
	class BSONFormatter : public IFormatter
	{
	public:
		MappingNode LoadFile(const std::filesystem::path& path) override;
		MappingNode LoadContent(const std::string& content) override { throw std::runtime_error("BSON is a binary-only format."); }
		MappingNode LoadContent(const std::vector<uint8_t>& content) override;

		bool SaveFile(const MappingNode& node, const std::filesystem::path& path) override;
		void SaveContent(const MappingNode& node, std::string& out_content) { throw std::runtime_error("BSON is a binary-only format."); }
		void SaveContent(const MappingNode& node, std::vector<uint8_t>& out_content) override;

		bool StringContent() const override { return false; };
	};
}

#endif // !XE_BSONFORMATTER_H