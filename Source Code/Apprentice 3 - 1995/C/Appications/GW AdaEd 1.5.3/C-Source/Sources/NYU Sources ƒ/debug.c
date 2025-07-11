/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */
#ifndef EXPORT
#ifdef gargar
#include "hdr.h"
#include "ada.h"
#include "adalex.h"
#include "reduce.h"
#include "pspans.h"

/*
 *		DEBUGGING PROCEDURES (for use with dbx)
 */

char *zkind_str(struct ast *);
int zastcount(struct ast *);

void zpdeadnode(struct ast *node)							/*;zpdeadnode*/
{
	printf("Node 0x%x %d, Kind %d %s\n",node,node,
	    node->kind,zkind_str(node));
	printf("link: 0x%x %d\n",((struct tmpnode *)node)->link,
	    ((struct tmpnode *)node)->link);
}

void zpnod_p(struct ast *node)								/*;zpnod*/
{
	short i,n;
	struct two_pool *listptr;

	if (node == NULL) {
		printf("node is NULL\n");
		return;
	}
	printf("Node 0x%x %d, Kind %d %s\n",node,node,
	    node->kind,zkind_str(node));
	if (isast_node[node->kind]) {
		n = zastcount(node);
		printf("astcount %d	 ",n);
		for (i=0; i<n; i++)
			printf("0x%x ",node->links.subast[i]);
		printf("\n");
	}
	else if (islist_node[node->kind]) {
		printf("list: ");
		if (node->links.list != NULL) {
			listptr = node->links.list;
			do {
				listptr = listptr->link;
				printf("0x%x ",listptr->val.node);
			} while (listptr != node->links.list);
			printf("\n");
		}
	}
	else if (node->kind == AS_SIMPLE_NAME) {
		printf("val	 %s\n",namelist(node->links.val));
	}
	if (is_terminal_node_p(node->kind) || node == OPT_NODE)
		printf("span line: %d, span col: %d \n",
		    node->span.line,node->span.col);
#ifdef TODO
	-- else retrieve and print spans info.
#endif
	    return;
}

int zastcount(struct ast *node)							/*;zastcount*/
{
	short i;

	i = 1;
	while( i < MAX_AST ) {
		if (node->links.subast[i] == NULL) return(i);
		i++;
	}
	return(MAX_AST);
}

void zpnlist(struct two_pool *listptr)					/*;zpnlist*/
{
	/* dump list of pointers (to nodes) */
	struct two_pool *bottom;
	if (listptr == NULL) {
		printf("null list\n");
		return;
	}
	else {
		bottom = listptr;
		do {
			listptr = listptr->link;
			printf("0x%x ",listptr->val.node);
		} while (listptr != bottom);
	}
}

/* Getlabels: Return the list of labels corresponding to a given node. If
   The map is not defined for a node, NULL is returned. */

/* this function is defined as dump_labels in labels.c (nodestolabelstable) 
zplabs(node)
struct ast *node;
{
    struct labelsmap *tmp;
    
    for (tmp = nodetolabelstable[labelshash(node)]; tmp != NULL && 
	tmp->node != node; tmp = tmp->link);
    zpnlist(tmp->list);
}
*/

void zprsstack(struct prsstack *p)							/*;zprsstack*/
{
	/* dump contents (addresses) of parse stack */
	while (p != NULL) {
		printf("0x%x ",p);
		p = p->prev;
	}
	printf("\n");

}

void zprssym(struct prsstack *p)							/*;zprssym*/
{
	/* dump symbols of parse stack */
	while (p != NULL) {
		printf("%s ",TOKSTR(p->symbol));
		p = p->prev;
	}
	printf("\n");

}

void zpstastack(struct two_pool *s)							/*;zpstastack*/
{
	int	    count = 0;
	while (s != NULL)
	{
		count++ ;
		printf ("%d %s", s -> val.state,
		    ( ((count%10) == 0) ? "\n":",") );
		s = s -> link;
	}
	printf (" ======>size = %d<====== \n",count);
}

void zprsnod(struct prsstack *p)							/*;zprsnod*/
{
	/* dump an entry on the parse stack */
	if (p == NULL)
		printf("null pointer\n");
	else if ISTOKEN(p) {
		struct token *tok = p->ptr.token;

		printf("%s ",TOKSTR(p->symbol));
		printf("%s\n",namelist(tok->index));
		printf(" line %d  col %d \n",tok->loc.line, tok->loc.col);
	}
	else { /* ast node (nonterminal) */
		printf("%s ",TOKSTR(p->symbol));
		printf(" ast node: %d\n",p->ptr.ast);
		if (p->ptr.ast != NULL)
			zpnod(p->ptr.ast);
	}
}

void zdnstack(struct ast *dead)							/*;zdnstack*/
{
	/* dump contents of dead node stack */
	printf("Deadnode_Stack ");
	while (dead != NULL) {
		printf("0x%x ",dead);
		dead = ((struct tmpnode *)dead)->link;
	}
	printf("End_of_Stack\n");

}

/*
 * The text of kind_str that follows is generated by a spitbol program
 */
char *zkind_str(struct ast *node)							/*;kind_str*/
{
	int as;

	static char *as_names[] = {
		"pragma",
		"arg",
		"obj_decl",
		"const_decl",
		"num_decl",
		"type_decl",
		"subtype_decl",
		"subtype_indic",
		"derived_type",
		"range",
		"range_attribute",
		"constraint",
		"enum",
		"int_type",
		"float_type",
		"fixed_type",
		"digits",
		"delta",
		"array_type",
		"box",
		"subtype",
		"record",
		"component_list",
		"field",
		"discr_spec",
		"variant_decl",
		"variant_choices",
		"string",
		"simple_choice",
		"range_choice",
		"choice_unresolved",
		"others_choice",
		"access_type",
		"incomplete_decl",
		"declarations",
		"labels",
		"character_literal",
		"simple_name",
		"call_unresolved",
		"selector",
		"all",
		"attribute",
		"aggregate",
		"parenthesis",
		"choice_list",
		"op",
		"in",
		"notin",
		"un_op",
		"int_literal",
		"real_literal",
		"string_literal",
		"null",
		"name",
		"qualify",
		"new_init",
		"new",
		"statements",
		"statement",
		"null_s",
		"assignment",
		"if",
		"cond_statements",
		"condition",
		"case",
		"case_statements",
		"loop",
		"while",
		"for",
		"forrev",
		"block",
		"exit",
		"return",
		"goto",
		"subprogram_decl",
		"procedure",
		"function",
		"operator",
		"formal",
		"mode",
		"subprogram",
		"call",
		"package_spec",
		"package_body",
		"private_decl",
		"use",
		"rename_obj",
		"rename_ex",
		"rename_pack",
		"rename_sub",
		"task_spec",
		"task_type_spec",
		"task",
		"entry",
		"entry_family",
		"accept",
		"delay",
		"selective_wait",
		"guard",
		"accept_alt",
		"delay_alt",
		"terminate_alt",
		"conditional_entry_call",
		"timed_entry_call",
		"abort",
		"unit",
		"with_use_list",
		"with",
		"subprogram_stub",
		"package_stub",
		"task_stub",
		"separate",
		"exception",
		"except_decl",
		"handler",
		"others",
		"raise",
		"generic_function",
		"generic_procedure",
		"generic_package",
		"generic_formals",
		"generic_obj",
		"generic_type",
		"gen_priv_type",
		"generic_subp",
		"generic",
		"package_instance",
		"function_instance",
		"procedure_instance",
		"instance",
		"length_clause",
		"enum_rep_clause",
		"rec_rep_clause",
		"compon_clause",
		"address_clause",
		"any_op",
		"opt",
		"list",
		"range_expression",
		"arg_assoc_list",
		"private",
		"limited_private",
		"code",
		"line_no",
		"free",
		0	};
	as=node->kind;
	return ( (as < 145) ? as_names[as] : "INVALID");
}
#endif
#endif
