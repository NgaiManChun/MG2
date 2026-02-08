#pragma once
#include <unordered_map>
#include <string>

namespace MG {

	struct MG_RESOURCE_HEADER {
		static constexpr char MAGIC[4] = { 'M','G','P','K' };
		char version[8];
		unsigned int entryCount;
	};

	struct MG_RESOURCE_ENTRY {
		char name[256];
		size_t size;
	};

	class MGResource {
	public:
		struct ResourceFile {
			unsigned char* data;
			size_t size;

			operator bool() const {
				return size;
			}
		};
		static MGResource Load(const char* filename);
		static constexpr const char MODEL_VERSION[8] = "1.0";
	private:
		std::unordered_map<std::string, ResourceFile> m_Files;
	public:
		MGResource(){}
		MGResource(const char* filename);
		void Add(const char* filename, const char* rename = nullptr);
		void Remove(const char* filename);
		ResourceFile GetFile(const char* filename);
		const std::unordered_map<std::string, ResourceFile>& GetAllFiles() { return m_Files; }
		void Write(const char* filename);
		void Release();
	};

} // namespace MG