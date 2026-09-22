#ifndef FLAGPARSER_H
#define FLAGPARSER_H

#include "types.h"

typedef enum {
	FLAG_ARG_NONE,
	FLAG_ARG_REQUIRED,
	FLAG_ARG_OPTIONAL
} fp_flag_type;

typedef struct {
	const char *sname;
	const char *lname;

	fp_flag_type arg_type;
	const char *value_type;
	const char *default_value;

	const char *description;

	b8 hidden;
} fp_flag;

typedef struct {
	const fp_flag *arg;
	const char *value;
} fp_parsed_flag;

typedef struct {
	int count;
	char **values;
} fp_flag_positions;

typedef struct {
	fp_parsed_flag *args;
	u32 arg_count;

	fp_flag_positions positions;

	const char *error;
} fp_result;

typedef struct {
	const char *program_name;
	const char *version;
	const char *description;

	const fp_flag *args;
	u32 arg_count;
} fp_config;

b8 fp_flag_parse(const fp_config *config, int argc, char **argv, fp_result *result);
void fp_flag_result_free(fp_result *result);

const fp_parsed_flag *fp_get_flag(const fp_result *result, const char *lname);
b8 fp_has_flag(const fp_result *result, const char *lname);

void fp_print_help(const fp_config *config);
void fp_print_usage(const fp_config *config);
void fp_print_error(const fp_config *config, const char *message);

#endif
