/*========================================================

 XEMarkup - JSON Formatter
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

#ifndef XE_JSONFORMATTER_H
#define XE_JSONFORMATTER_H

#include <XEMarkup/IFormatter.h>

namespace xe
{
	class JSONFormatter : public IFormatter
	{
	public:
		JSONFormatter() = default;
		JSONFormatter(const bool usePrettyFormat) : m_usePrettyFormat(usePrettyFormat) {}

		MappingNode LoadFile(const std::filesystem::path& path) override;
		MappingNode LoadContent(const std::string& content) override;
		MappingNode LoadContent(const std::vector<uint8_t>& content) override;

		bool SaveFile(const MappingNode& node, const std::filesystem::path& path) override;
		void SaveContent(const MappingNode& node, std::string& out_content) override;
		void SaveContent(const MappingNode& node, std::vector<uint8_t>& out_content) override;

		bool GetUsePrettyFormat() const { return m_usePrettyFormat; }
		void SetUsePrettyFormat(const bool usePrettyFormat) { m_usePrettyFormat = usePrettyFormat; }

	private:
		bool m_usePrettyFormat = false;
	};
}

#endif // !XE_JSONFORMATTER_H