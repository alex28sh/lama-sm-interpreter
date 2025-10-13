// Ensures the linker defines __start_custom_data and __stop_custom_data
// by contributing at least one input section named "custom_data".
// This file is intentionally tiny and has no runtime impact.
#ifdef __GNUC__
__attribute__((section("custom_data"), used))
static const unsigned char lama_custom_data_anchor[1] = {0};
#endif
