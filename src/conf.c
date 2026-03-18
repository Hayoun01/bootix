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
	if (!entry)
		return (NULL);
	memset(entry, 0, sizeof(cnf_entry));

	// parsing key
	while ((*cnf)[i] &&strchr("\t\n =", (*cnf)[i]) == NULL){
		i++;
	}
	
	entry->key = malloc(i + 1);
	if (entry->key == NULL) {
		free(entry);
		return (NULL);
	}
	memcpy(entry->key, *cnf, i);
	entry->key[i] = '\x00';
	*cnf+=i;

	// skipping = and spaces
	while (**cnf && strchr("\t =", **cnf) != NULL)
		(*cnf)++;


	if (**cnf == '\n' || **cnf == '\0')
		return (entry);

	
	// parsing value
	i = 0;
	if (**cnf == '"' || **cnf == '\''){
		sep = **cnf;
		(*cnf)++;
		while ((*cnf)[i] && (*cnf)[i] != sep)
			i++;
	} else {
		while ((*cnf)[i] && strchr("\t\n =", (*cnf)[i]) == NULL){
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
	if (head == NULL)
		return (NULL);
	memset(head, 0, sizeof(cnf_namespace));
	head->ns = strdup("default");

	return (head);
}

char	*cnf_parse_namespace(char **cnf){
	uint32_t i = 0;
	char *ns;

	(*cnf)++;
	skip_whitespaces(cnf);
	while ((*cnf)[i] && (*cnf)[i] != ']' && (*cnf)[i] != '\n'){
		i++;
	}
	ns = malloc(i + 1);
	if (!ns)
		return (NULL);
	memcpy(ns, *cnf, i);
	ns[i] = '\0';
	if (*cnf[i] == ']')
		(*cnf)++;
	return (ns);
}

cnf_namespace	*cnf_parse(char *cnf){
	cnf_namespace *head = cnf_init_namespace();
	cnf_namespace *it = head;
	it->entry = NULL;
	it->next = NULL;
	cnf_entry *centry = NULL;

	while (*cnf != '\x00'){
		skip_whitespaces(&cnf);
		skip_comments(&cnf);
		if (*cnf == '\x00')
			break;
		if(*cnf == '\n'){
			cnf++;
			continue;
		}
		if (*cnf == '['){		// parsing namespaces
			it->next = malloc(sizeof(cnf_namespace));
			memset(it->next, 0, sizeof(cnf_namespace));
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
#ifdef DBGX
	print_cnf(head);
#endif
	return (head);
}


// Searching 
cnf_namespace	*cnf_search_namespace(cnf_namespace *ns, char *dns){
	cnf_namespace *it = ns;
	while (it != NULL){
		if (strcmp(dns, it->ns) == 0)
			return (it);
		it = it->next;
	}
	return (NULL);
}

cnf_entry	*cnf_search_entries(cnf_entry *entries, char *dentry){
	cnf_entry *it = entries;
	while (it != NULL){
		if (strcmp(dentry, it->key) == 0)
			return (it);
		it = it->next;
	}
	return (NULL);
}


// jantorial
void cnf_free(cnf_namespace *ns){
	while (ns != NULL){
		cnf_entry *en = ns->entry;
		while (en != NULL){
			cnf_entry *next = en->next;
			free(en->key);
			free(en->val);
			free(en);
			en = next;
		}
		cnf_namespace *next_ns = ns->next;
		free(ns->ns);
		free(ns);
		ns = next_ns;
	}
}

// other utils
cnf_namespace *cnf_clone(cnf_namespace *ns){
	cnf_namespace *clone = malloc(sizeof(cnf_namespace));

	clone->ns = strdup(ns->ns);
	clone->next = NULL;

	// cloning entries
	cnf_entry *en = ns->entry;
	clone->entry = malloc(sizeof(cnf_entry));
	cnf_entry *cen = clone->entry;
	while (en != NULL){
		cen->key = strdup(en->key);
		cen->val = strdup(en->key);
		if (en->next != NULL)
			cen->next = malloc(sizeof(cnf_entry));
		else 
			cen->next = NULL;
		cen = cen->next;
		en = en->next;
	}
	
	return (clone);
}
