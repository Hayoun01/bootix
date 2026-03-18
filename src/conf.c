#include "../inc/bootix.h"
// a toml like parser

#ifdef DBGX
void print_cnf(cnf_namespace *ns){
	while (ns != NULL){
		printf("namespace: %s\n", ns->ns);
		cnf_entry *en = ns->entry;
		while (en != NULL){
			printf("\t%s -> %s\n", en->key, en->val);
			en = en->next;
		}
		ns = ns->next;
	}
}
#endif

void skip_nl(char **s){
	while (**s != '\n' && **s != '\0')
		(*s)++;
	if (**s == '\n')
		(*s)++;
}

void skip_whitespaces(char **s){
	while (**s == ' ' || **s == '\t' ||**s == '\r')
		(*s)++;
}

void skip_comments(char **s){
	if (**s == '#'){
		while (**s != '\n' && **s != '\0'){
			(*s)++;
		}
	}
}


// parsing entries in a namespace
static cnf_entry	*cnf_parse_entry(char **cnf){
	uint32_t i = 0;
	char	sep;
	cnf_entry *entry = malloc(sizeof(cnf_entry));

	// parsing key
	while (strchr("\t\n =", (*cnf)[i]) == NULL){
		i++;
	}

	entry->key = malloc(i + 1);
	memcpy(entry->key, *cnf, i);
	entry->key[i] = '\x00';
	*cnf+=i;
	// skipping = and spaces
	while (strchr("\t =", **cnf) != NULL && **cnf != '\0')
		(*cnf)++;


	if (**cnf == '\n' || **cnf == '\0')
		return (entry);

	
	// parsing value
	i = 0;
	if (*cnf[i] == '"' || *cnf[i] == '\''){
		sep = *cnf[i];
		(*cnf)++;
		while (*cnf[i] != sep && *cnf[i] != '\0')
			i++;
	} else {
		while (strchr("\t\n =", (*cnf)[i]) == NULL  && (*cnf)[i] != '\0'){
			i++;
		}
	}
	entry->val = malloc(i + 1);
	memcpy(entry->val, *cnf, i);
	entry->val[i] = '\x00';
	*cnf+=i;

	return (entry);
}
static cnf_namespace	*cnf_init_namespace(){
	cnf_namespace *head = malloc(sizeof(cnf_namespace));
	head->ns = strdup("default");

	return (head);
}

char	*cnf_parse_namespace(char **cnf){
	uint32_t i = 0;
	char *ns;

	(*cnf)++;
	skip_whitespaces(cnf);
	while (*cnf[i] != ']' && *cnf[i] != 0 && *cnf[i] != '\n'){
		i++;
	}
	ns = malloc(i + 1);
	memcpy(ns, cnf, i);
	if (*cnf[i] == ']')
		(*cnf)++;
	return (ns);
}

cnf_namespace	*cnf_parse(char *cnf){
	cnf_namespace *head = cnf_init_namespace();
	cnf_namespace *it = head;
	cnf_entry *centry = NULL;

	while (*cnf != '\x00'){
		skip_whitespaces(&cnf);
		skip_comments(&cnf);
		if (*cnf == '\x00')
			break;
		if(*cnf == '\n')
			continue;
		if (*cnf == '['){		// parsing namespaces
			it->next = malloc(sizeof(cnf_namespace));
			it = it->next;
			it->ns = cnf_parse_namespace(&cnf);
			it->entry = NULL;
			it->next = NULL;
		} else {
			centry = cnf_parse_entry(&cnf);
			if (centry != NULL){
				centry->next = it->entry;
				it->entry = centry;
			}
		}
		skip_nl(&cnf);
	}
	print_cnf(head);

	return (head);
}

