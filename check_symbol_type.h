#ifndef CHECK_SYMBOL_TYPE_H
#define CHECK_SYMBOL_TYPE_H

int is_var_type(char *str)
{
	int i = 0;

	i = i || (strcmp(str, "void") == 0 ? 1 : 0);
	i = i || (strcmp(str, "int") == 0 ? 1 : 0);
	i = i || (strcmp(str, "double") == 0 ? 1 : 0);
	i = i || (strcmp(str, "float") == 0 ? 1 : 0);
	i = i || (strcmp(str, "char") == 0 ? 1 : 0);
	i = i || (strcmp(str, "unsigned") == 0 ? 1 : 0);
	i = i || (strcmp(str, "long") == 0 ? 1 : 0);

	return i;
}

int is_identifier(char *str)
{
	int res1, res2, res3, i = 1;

	res1 = (int)(str[0] - 'a');
	res2 = (int)(str[0] - 'A');
	if (str[0] != '_' && (res1 < 0 || res1 > 25)
		&& (res2 < 0 || res2 > 25)) {
		return 0;
	}

	while (str[i] != '\0') {
		res1 = (int)(str[i] - 'a');
		res2 = (int)(str[i] - 'A');
		res3 = (int)(str[i] - '0');

		if (str[i] != '_' && (res1 < 0 || res1 > 25) &&
			(res2 < 0 || res2 > 25) && (res3 < 0 || res3 > 9)) {
			return 0;
		}
		++i;
	}
	return 1;
}

int is_number_literal(char *str)
{
	int i = 0;
	int res;

	while (str[i] != '\0') {
		res = str[i] - '0';
		if (res < 0 || res > 9)
			return 0;

		++i;
	}

	return 1;
}

#endif // CHECK_SYMBOL_TYPE_H
