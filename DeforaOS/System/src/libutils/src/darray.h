/* darray.h */



#ifndef ___DARRAY_H
# define ___DARRAY_H


/* types */
typedef struct _DArray {
	int size;
	int count;
	void ** array;
} DArray;


/* functions */
/* darray */
/* darray_new
 * PRE
 * POST	creates an empty DArray
 * RETURNS
 * 	NULL	failure
 * 	else	the DArray */
DArray * darray_new(void);
/* darray_delete
 * PRE
 * POST	d is free'd if valid */
void darray_delete(DArray * d);

/* returns */
/* darray_count
 * PRE	d is a valid DArray
 * POST
 * RETURNS
 * 	any	the number of elements contained in the array */
int darray_count(DArray * d);
void * darray_get(DArray * d, int n);

/* sets */
int darray_set(DArray * d, int n, void * data);

/* useful */
/* darray_merge
 * PRE	d is a valid DArray
 * POST	trailing NULLs in the data array are dealloc' */
void darray_merge(DArray * d);

#endif /* !___DARRAY_H */
