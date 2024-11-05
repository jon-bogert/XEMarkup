/*========================================================

 XEMarkup - IFormatter
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

#ifndef XE_IFORMATTER_H
#define XE_IFORMATTER_H

#include "MappingNode.h"

#include <filesystem>
#include <string>
#include <vector>

namespace xe
{
	class IFormatter
	{
	public:
		virtual MappingNode LoadFile(const std::filesystem::path& path) = 0;
		virtual MappingNode LoadContent(const std::string& content) = 0;
		virtual MappingNode LoadContent(const std::vector<uint8_t>& content) = 0;

		virtual bool SaveFile(const MappingNode& node, const std::filesystem::path& path) = 0;
		virtual void SaveContent(const MappingNode& node, std::string& out_content) = 0;
		virtual void SaveContent(const MappingNode& node, std::vector<uint8_t>& out_content) = 0;

		virtual bool StringContent() const { return true; }; // override for binary based file formats
	};
}

#endif // !XE_IFORMATTER_H