#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef void exdata_free(int idx, void *ex_ptr, void *arg);

struct exdata_schema;

struct exdata_schema *exdata_schema_new();
void exdata_schema_free(struct exdata_schema *schemas);

int exdata_schema_new_index(struct exdata_schema *schema, const char *name, exdata_free *free_func,void *free_arg);

int exdata_schema_get_idx_by_name(struct exdata_schema *schema, const char *name);

struct exdata_runtime;

struct exdata_runtime *exdata_runtime_new(struct exdata_schema *schemas);
void exdata_runtime_free(struct exdata_runtime *rt);
void exdata_runtime_reset(struct exdata_runtime *rt);//call free_func, and set ex_ptr to NULL

int exdata_set(struct exdata_runtime *rt, int idx, void *ex_ptr);
void *exdata_get(struct exdata_runtime *rt, int idx);


#ifdef __cplusplus
}
#endif