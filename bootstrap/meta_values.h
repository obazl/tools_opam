/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
typedef struct obzl_meta_values obzl_meta_values;
#if DEBUG_TRACE
void dump_values(int indent,obzl_meta_values *values);
#endif
obzl_meta_values *obzl_meta_values_new_tokenized(char *valstr);
obzl_meta_values *obzl_meta_values_new_copy(obzl_meta_values *values);
obzl_meta_values *obzl_meta_values_new(char *valstr);
typedef char *obzl_meta_value;
obzl_meta_value *obzl_meta_values_nth(obzl_meta_values *_values,int _i);
int obzl_meta_values_count(obzl_meta_values *_values);
#define EXPORT
struct obzl_meta_values {
    UT_array *list;             /* list of strings  */
};
#define INTERFACE 0
