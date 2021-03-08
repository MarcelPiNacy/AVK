#include <cstdio>
#include <cassert>
#include <string>
#include <Windows.h>



const char* solution_dir;
constexpr auto buffer_a_size = 1 << 12;
char buffer_a[buffer_a_size];



int main(int argc, const char** args)
{
	if (argc != 2)
	{
		puts("Missing solution directory!");
		return -1;
	}

	solution_dir = args[1];
	std::string resource;
	std::string callbacks;
	auto k = snprintf(buffer_a, buffer_a_size, "%savk\\internal\\windows-specific\\resource_template_a.txt", solution_dir);
	assert(k > 0);
	FILE* file;
	fopen_s(&file, buffer_a, "rb");
	assert(file != nullptr);
	fseek(file, 0, SEEK_END);
	resource.resize((size_t)ftell(file));
	fseek(file, 0, SEEK_SET);
	fread(resource.data(), 1, resource.size(), file);
	resource.append("\n\n");
	fclose(file);
	std::string all_header =
		"#pragma once\n#include \"../internal/main_array.h\"\n\n";
	WIN32_FIND_DATAA data;
	k = snprintf(buffer_a, buffer_a_size, "%savk\\algorithms\\*.cpp", solution_dir);
	assert(k > 0);
	auto finder = FindFirstFileA(buffer_a, &data);
	assert(finder != INVALID_HANDLE_VALUE);
	uint64_t counter = 500;
	do
	{
		printf("Found sort file \"%s\".\n", data.cFileName);
		auto end = data.cFileName + strlen(data.cFileName);
		auto i = std::find(data.cFileName, end, '.');
		assert(i != end);
		*i = '\0';
		k = snprintf(buffer_a, buffer_a_size, "#define IDM_SORT_%s %llu\n", data.cFileName, counter);
		assert(k > 0);
		resource.append(buffer_a);
		++counter;
		k = snprintf(buffer_a, buffer_a_size, "void %s(main_array array);\n", data.cFileName);
		assert(k > 0);
		all_header.append(buffer_a);
		k = snprintf(buffer_a, buffer_a_size,
			"case IDM_SORT_%s:\n"
			"\talgorithm_thread::assign_body(%s);\n"
			"\tbreak;\n", data.cFileName, data.cFileName);
		assert(k > 0);
		callbacks.append(buffer_a);
	} while (FindNextFileA(finder, &data));
	FindClose(finder);
	k = snprintf(buffer_a, buffer_a_size, "%savk\\algorithms\\all.h", solution_dir);
	assert(k > 0);
	fopen_s(&file, buffer_a, "wb");
	assert(file != nullptr);
	fwrite(all_header.data(), 1, all_header.size(), file);
	fclose(file);
	k = snprintf(buffer_a, buffer_a_size, "%savk\\internal\\windows-specific\\resource_template_b.txt", solution_dir);
	assert(k > 0);
	fopen_s(&file, buffer_a, "rb");
	assert(file != nullptr);
	fseek(file, 0, SEEK_END);
	auto prior_size = resource.size();
	auto file_size = (size_t)ftell(file);
	resource.resize(prior_size + file_size);
	fseek(file, 0, SEEK_SET);
	fread(resource.data() + prior_size, 1, file_size, file);
	fclose(file);
	k = snprintf(buffer_a, buffer_a_size, "%savk\\internal\\windows-specific\\resource.h", solution_dir);
	assert(k > 0);
	fopen_s(&file, buffer_a, "wb");
	assert(file != nullptr);
	fwrite(resource.data(), 1, resource.size(), file);
	fclose(file);
	k = snprintf(buffer_a, buffer_a_size, "%savk\\internal\\windows-specific\\sort_callbacks.inl", solution_dir);
	assert(k > 0);
	fopen_s(&file, buffer_a, "wb");
	assert(file != nullptr);
	fwrite(callbacks.data(), 1, callbacks.size(), file);
	fclose(file);
	return 0;
}