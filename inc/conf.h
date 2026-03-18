#ifndef CONF_H
#define CONF_H

typedef struct cnf_entry {
	char			*key;
	char			*val;
	struct cnf_entry	*next;
} cnf_entry;

typedef struct cnf_namespace{
	char			*ns;
	cnf_entry		*entry;
	struct cnf_namespace	*next;
}cnf_namespace;

cnf_namespace	*cnf_parse(char *cnf);
cnf_namespace	*cnf_search_namespace(cnf_namespace *ns, char *dns);
void cnf_free(cnf_namespace *ns);
cnf_entry	*cnf_search_entries(cnf_entry *entries, char *dentry);
cnf_namespace *cnf_clone(cnf_namespace *ns);


#endif
