/* slist.h */




#ifndef ___SLIST_H
# define ___SLIST_H


/* types */
typedef enum _SListType { ST_HEAD, ST_CELL } SListType;
typedef struct _SList {
	SListType type;
	void * data;
	struct _SList * next;
} SList;


/* functions */
/* slist_new
 * PRE
 * POST creates an empty SList
 * 	NULL	failure
 * 	else	the new SList */
SList * slist_new(void);

/* slist_delete
 * PRE
 * POST	frees a SList (not the data) */
void slist_delete(SList * slist);

#endif /* !___SLIST_H */
