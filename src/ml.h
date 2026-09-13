#ifndef ML_H_
#define ML_H_

#include "task.h"
#include "nob.h"
#include "md.h"

typedef enum {
    ML_OP_SET_PROP,
    ML_OP_SET_PRIORITY,
    ML_OP_SET_TAGS,
    ML_OP_CLOSE,
    ML_OP_OPEN,
    ML_OP_TAG,
    ML_OP_UNTAG,
} Ml_Op_Kind;

typedef struct {
    Ml_Op_Kind kind;
    union {
        Property prop;
        long integer;
        String_View sv;
        Tags tags;
    } as;
    String_View src;
} Ml_Op;

void print_ml_op(Ml_Op op);

Ml_Op ml_op(Ml_Op_Kind kind, String_View src);
Ml_Op ml_op_store_prop(Ml_Op op, Property prop);
Ml_Op ml_op_store_integer(Ml_Op op, long value);
Ml_Op ml_op_store_sv(Ml_Op op, String_View value);
Ml_Op ml_op_store_tags(Ml_Op op, Tags tags);

#define ml_op_set_prop(src, prop)      ml_op_store_prop(ml_op(ML_OP_SET_PROP, (src)), (prop))
#define ml_op_set_priority(src, value) ml_op_store_integer(ml_op(ML_OP_SET_PRIORITY, (src)), (value))
#define ml_op_set_tags(src, tags)      ml_op_store_tags(ml_op(ML_OP_SET_TAGS, (src)), (tags))
#define ml_op_close(src)               ml_op(ML_OP_CLOSE, (src))
#define ml_op_open(src)                ml_op(ML_OP_OPEN, (src))
#define ml_op_tag(src, tag)            ml_op_store_sv(ml_op(ML_OP_TAG, (src)), (tag))
#define ml_op_untag(src, tag)          ml_op_store_sv(ml_op(ML_OP_UNTAG, (src)), (tag))

typedef struct {
    Ml_Op *items;
    size_t count;
    size_t capacity;
} Modyfying_Language;

bool compile_modyfying_language(String_View original_src, String_View *src, Modyfying_Language *ml);

bool eval_modyfying_language(Task *task, Modyfying_Language ml);

#endif // ML_H_
