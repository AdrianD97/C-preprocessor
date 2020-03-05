#ifndef SYMBOLS_VALUES_H
#define SYMBOLS_VALUES_H

int get_symbol_value(char *value, char *final_value)
{
	char words[LINE_WORDS][WORD_SIZE];
	int nr_words;
	int i, result;
	void *val;
	char *copy_val, *f_val;
	char *delm = "\t ";

	if (value[0] == '\0') {
		final_value[0] = '\0';
		return SUCCESS;
	}

	if (value[0] == '?') {
		strcpy(final_value, value + 1);
		strcat(final_value, " ");
		return SUCCESS;
	}

	copy_val = (char *)malloc((strlen(value) + 1) * sizeof(char));
	if (!copy_val) {
		printf("Error: Can not alloc memory "
			"for \'copy_val\' variable.\n");
		return -ENOMEM;
	}

	strcpy(copy_val, value);

	split_line(copy_val, words, &nr_words, delm);
	if (nr_words == 0)
		final_value[0] = '\0';

	for (i = 0; i < nr_words; ++i) {
		if (is_identifier(words[i])) {
			f_val = (char *)malloc(MAPPING_SIZE * sizeof(char));
			if (!f_val) {
				printf("Error: Can not alloc memory "
					"for \'f_val\' variable.\n");
				free(copy_val);
				return -ENOMEM;
			}
			f_val[0] = '\0';

			val = get(hash_table, (void *)words[i]);
			if (!val) {
				strcat(f_val, words[i]);
				strcat(f_val, " ");
			} else {
				result = get_symbol_value(val, f_val);
				if (result != SUCCESS) {
					free(f_val);
					free(copy_val);
					return result;
				}
			}

			strcat(final_value, f_val);
			free(f_val);
		} else {
			strcat(final_value, words[i]);
			strcat(final_value, " ");
		}
	}

	free(copy_val);
	return SUCCESS;
}

#endif // SYMBOLS_VALUES_H
