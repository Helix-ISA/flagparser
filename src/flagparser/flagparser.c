#include "flagparser/flagparser.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const fp_flag *find_long_option(
	const fp_config *config,
	const char *name
)
{
	for (u32 i = 0; i < config->flag_count; i++) {
		const fp_flag *option = &config->flags[i];

		if (option->lname && strcmp(option->lname, name) == 0) {
			return option;
		}
	}

	return NULL;
}

static const fp_flag *find_short_option(
	const fp_config *config,
	const char *name
)
{
	for (u32 i = 0; i < config->flag_count; i++) {
		const fp_flag *option = &config->flags[i];

		if (option->sname && strcmp(option->sname, name) == 0) {
			return option;
		}
	}

	return NULL;
}

static b8 add_parsed_arg(
	fp_result *result,
	const fp_flag *arg,
	const char *value
)
{
	fp_parsed_flag *new_options = realloc(
			result->flags,
			(result->flag_count + 1) * sizeof(*result->flags)
	);

	if (!new_options)
		return failure;

	result->flags = new_options;

	result->flags[result->flag_count].flag = arg;
	result->flags[result->flag_count].value = value;

	result->flag_count++;

	return success;
}

static b8 add_position(fp_result *result, char *value)
{
	char **new_values = realloc(
			result->positions.values,
			(result->positions.count + 1) *
			sizeof(*result->positions.values)
	);

	if (!new_values)
		return failure;

	result->positions.values = new_values;

	result->positions.values[result->positions.count] = value;
	result->positions.count++;

	return success;
}

static b8 parse_error(fp_result *result, const char *message)
{
	result->error = message;
	return failure;
}

static b8 result_has_arg(const fp_result *result, const fp_flag *arg)
{
	for (size_t i = 0; i < result->flag_count; i++) {
		if (result->flags[i].flag == arg)
			return true;
	}

	return false;
}

b8 fp_flag_parse(const fp_config *config, int argc, char **argv, fp_result *result)
{
	if (!config || !result)
		return failure;

	memset(result, 0, sizeof(*result));

	b8 end_of_options = false;

	for (int i = 1; i < argc; i++) {
		char *arg = argv[i];

		if (end_of_options) {
			if (!add_position(result, arg))
				return parse_error(result, "out of memory");

			continue;
		}

		if (strcmp(arg, "--") == 0) {
			end_of_options = true;
			continue;
		}

		if (strcmp(arg, "-") == 0) {
			if (!add_position(result, arg))
				return parse_error(result, "out of memory");

			continue;
		}

		if (strncmp(arg, "--", 2) == 0) {
			char *name = arg + 2;

			if (*name == '\0')
				continue;

			char *equals = strchr(name, '=');
			const char *inline_value = NULL;

			if (equals) {
				*equals = '\0';
				inline_value = equals + 1;
			}

			const fp_flag *arg = find_long_option(config, name);

			if (equals)
				*equals = '=';

			if (!arg)
				return parse_error(result, "unknown option");

			const char *value = NULL;

			if (arg->flag_type == FLAG_ARG_NONE) {
				if (inline_value)
					return parse_error(
						result,
						"option does not accept a value"
					);
			} else if (arg->flag_type == FLAG_ARG_REQUIRED) {
				if (inline_value) {
					value = inline_value;
				} else {
					if (i + 1 >= argc) {
						return parse_error(
							result,
							"option requires a value"
						);
					}

					value = argv[++i];
				}
			} else if (arg->flag_type == FLAG_ARG_OPTIONAL) {
				if (inline_value) {
					value = inline_value;
				} else if (i + 1 < argc && argv[i + 1][0] != '-') {
					value = argv[++i];
				}
			}

			if (!add_parsed_arg(result, arg, value))
				return parse_error(result, "out of memory");

			continue;
		}

		if (arg[0] == '-' && arg[1] != '\0') {
			char *sargs = arg + 1;

			while (*sargs) {
				char *name = sargs++;

				const fp_flag *arg = find_short_option(config, name);

				if (!arg) {
					return parse_error(
						result,
						"unknown option"
					);
				}

				const char *value = NULL;

				if (arg->flag_type == FLAG_ARG_NONE) {
				} else {
					if (*sargs != '\0') {
						value = sargs;

						sargs += strlen(sargs);
					} else if (arg->flag_type == FLAG_ARG_REQUIRED) {
						if (i + 1 >= argc) {
							parse_error(
								result,
								"option requires a value"
							);
						}

						value = argv[++i];
					} else if (arg->flag_type == FLAG_ARG_OPTIONAL) {
						if (i + 1 < argc && argv[i + 1][0] != '-') {
							value = argv[++i];
						}
					}
				}

				if (!add_parsed_arg(result, arg, value))
					return parse_error(result, "out of memory");

				if (arg->flag_type != FLAG_ARG_NONE)
					break;
			}

			continue;
		}
	
		if (!add_position(result, arg))
			return parse_error(result, "out of memory");
	}

	for (u32 i = 0; i < config->flag_count; i++) {
		const fp_flag *arg = &config->flags[i];

		if (!arg->default_value)
			continue;

		if (result_has_arg(result, arg))
			continue;

		if (!add_parsed_arg(result, arg, arg->default_value))
			return parse_error(result, "out of memory");
	}

	return true;
}

void fp_result_free(fp_result *result)
{
	if (!result)
		return;

	free(result->flags);
	free(result->positions.values);

	memset(result, 0, sizeof(*result));
}

const fp_parsed_flag *fp_get_flag(const fp_result *result, const char *lname)
{
	if (!result || !lname)
		return NULL;

	for (u32 i = 0; i < result->flag_count; i++) {
		const fp_parsed_flag *parsed = &result->flags[i];

		if (parsed->flag->lname && strcmp(parsed->flag->lname, lname) == 0)
			return parsed;
	}

	return NULL;
}

b8 fp_has_flag(const fp_result *result, const char *lname)
{
	return fp_get_flag(result, lname) != NULL;
}

void fp_print_help(const fp_config *config)
{
	if (!config)
		return;

	printf(
		"Usage: %s [OPTIONS]",
		config->program_name ? config->program_name : "program"
	);

	if (config->flag_count > 0)
		printf(" [ARGS...]");

	putchar('\n');
}

void fp_print_usage(const fp_config *config)
{
	if (!config)
		return;

	fp_print_help(config);

	if (config->description) {
		if (config->version) {
			printf("\n%s", config->description);
			printf(" (%s)\n", config->version);
		} else {
			printf("\n%s\n", config->description);
		}
	}


	printf("\nOptions:\n");

	u32 max_width = 0;

	for (u32 i = 0; i < config->flag_count; i++) {
		const fp_flag *arg = &config->flags[i];

		if (arg->hidden)
			continue;

		u32 width = 0;

		if (arg->sname)
			width += 1 + strlen(arg->sname);

		if (arg->sname && arg->lname)
			width += 2;

		if (arg->lname)
			width += 2 + strlen(arg->lname);

		if (arg->flag_type != FLAG_ARG_NONE && arg->value_type)
			width += 1 + strlen(arg->value_type);

		if (width > max_width)
			max_width = width;
	}
	
	for (u32 i = 0; i < config->flag_count; i++) {
		const fp_flag *arg = &config->flags[i];

		if (arg->hidden)
			continue;

		char buffer[512];
		u32 position = 0;

		if (arg->sname) {
			buffer[position++] = '-';

			u32 length = strlen(arg->sname);

			if (position + length >= sizeof(buffer))
				length = sizeof(buffer) - position - 1;

			memcpy(buffer + position, arg->sname, length);

			position += length;
		}

		if (arg->sname && arg->lname) {
			buffer[position++] = ',';
			buffer[position++] = ' ';
		}

		if (arg->lname) {
			buffer[position++] = '-';
			buffer[position++] = '-';

			u32 length = strlen(arg->lname);

			if (position + length >= sizeof(buffer))
				length = sizeof(buffer) - position - 1;

			memcpy(buffer + position, arg->lname, length);

			position += length;
		}

		if (arg->flag_type != FLAG_ARG_NONE && arg->value_type) {
			buffer[position++] = ' ';

			u32 length = strlen(arg->value_type);

			if (position + length >= sizeof(buffer))
				length = sizeof(buffer) - position - 1;

			memcpy(buffer + position, arg->value_type, length);

			position += length;
		}

		buffer[position] = '\0';

		printf("  %-*s", max_width, buffer);

		if (arg->description)
			printf("  %s", arg->description);

		if (arg->default_value)
			printf(" [default: %s]", arg->default_value);

		putchar('\n');
	}
}

void fp_print_error(const fp_config *config, const char *message)
{
	if (message)
		fprintf(stderr, "Error: %s\n", message);

	if (config)
		fp_print_usage(config);
}

