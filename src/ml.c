#include "ml.h"

int is_not_space(int x)
{
    return !isspace(x);
}

String_View chop_next_ml_token(String_View *src) {
    *src = sv_trim_left(*src);
    if (src->count == 0) return *src;
    return sv_chop_while(src, is_not_space);
}

bool compile_modyfying_language(String_View original_src, String_View *src, Modyfying_Language *ml)
{
    String_View token = chop_next_ml_token(src);
    if (sv_starts_with(token, SVLIT("."))) {
        if (token.count == 1) {
            report_compile_query_diagnostic(original_src, token, "ERROR: empty property\n");
            return false;
        }
        sv_chop_left(&token, 1);
        String_View assignment = chop_next_ml_token(src);
        if (!sv_eq(assignment, SVLIT("="))) {
            report_compile_query_diagnostic(original_src, token, "ERROR: assignment operator expected\n");
            return false;
        }
        String_View value = chop_next_ml_token(src);
        if (sv_eq(token, SVLIT("priority"))) {
            size_t checkpoint = temp_save();
            char *endptr      = NULL;
            const char *nptr  = temp_sv_to_cstr(value);
            long integer      = strtol(nptr, &endptr, 10);
            bool ok           = nptr < endptr && *endptr == '\0';
            temp_rewind(checkpoint);
            if (!ok) {
                report_compile_query_diagnostic(original_src, value, "ERROR: integer priority expected\n");
                return false;
            }
            da_append(ml, ml_op_set_priority(sv_from_parts(token.data, src->data - token.data), integer));
            return true;
        }
        if (sv_eq(token, SVLIT("tags"))) {
            Tags tags = {0};
            while (src->count) {
                String_View tag = chop_next_ml_token(src);
                if (!sv_starts_with(token, SVLIT(":"))) break;
                sv_chop_left(&tag, 1);
                da_append(&tags, tag);
            }
            da_append(ml, ml_op_set_tags(sv_from_parts(token.data, src->data - token.data), tags));
            return true;
        }
        Property prop = {.key = token, .value =  value};
        da_append(ml, ml_op_set_prop(sv_from_parts(token.data, src->data - token.data), prop));
        return true;
    }
    if (sv_eq(token, SVLIT("tag"))) {
        String_View tag = chop_next_ml_token(src);
        if (!sv_starts_with(token, SVLIT(":"))) {
            report_compile_query_diagnostic(original_src, token, "ERROR: tag expected\n");
            return false;
        }
        sv_chop_left(&tag, 1);
        da_append(ml, ml_op_tag(sv_from_parts(token.data, src->data - token.data), tag));
        return true;
    }
    if (sv_eq(token, SVLIT("untag"))) {
        String_View tag = chop_next_ml_token(src);
        if (!sv_starts_with(token, SVLIT(":"))) {
            report_compile_query_diagnostic(original_src, token, "ERROR: tag expected\n");
            return false;
        }
        sv_chop_left(&tag, 1);
        da_append(ml, ml_op_untag(sv_from_parts(token.data, src->data - token.data), tag));
        return true;
    }
    if (sv_eq(token, SVLIT("open"))) {
        da_append(ml, ml_op_open(sv_from_parts(token.data, src->data - token.data)));
        return true;
    }
    if (sv_eq(token, SVLIT("close"))) {
        da_append(ml, ml_op_close(sv_from_parts(token.data, src->data - token.data)));
        return true;
    }
    report_compile_query_diagnostic(original_src, token, "ERROR: ml expression expected\n");
    return false;
}

bool eval_modyfying_language(Task *task, Modyfying_Language ml) {
    bool changed = false;
    da_foreach(Ml_Op, op, &ml) {
        switch (op->kind) {
        case ML_OP_SET_PROP: {
            bool found = false;
            da_foreach(Property, prop, &task->properties) {
                if (sv_eq(prop->key, op->as.prop.key)) {
                    if (!sv_eq(prop->value, op->as.prop.value)) {
                        prop->value = op->as.prop.value;
                        changed = true;
                    }
                    found = true;
                    break;
                }
            }
            if (!found) {
                da_append(&task->properties, op->as.prop);
                changed = true;
            }
        } break;
        case ML_OP_SET_PRIORITY: {
            task->priority = op->as.integer;
            changed = true;
        } break;
        case ML_OP_SET_TAGS: {
            task->tags.count = 0;
            da_append_many(&task->tags, op->as.tags.items, op->as.tags.count);
            changed = true;
        } break;
        case ML_OP_CLOSE: {
            if (!sv_eq(task->status, SVLIT("CLOSED"))) {
                task->status = SVLIT("CLOSED");
                changed = true;
            }
        } break;
        case ML_OP_OPEN: {
            if (!sv_eq(task->status, SVLIT("OPEN"))) {
                task->status = SVLIT("OPEN");
                changed = true;
            }
        } break;
        case ML_OP_TAG: {
            bool found = false;
            da_foreach(String_View, tag, &task->tags) {
                if (sv_eq(*tag, op->as.sv)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                da_append(&task->tags, op->as.sv);
                changed = true;
            }
        } break;
        case ML_OP_UNTAG: {
            Tags tags = {0};
            da_foreach(String_View, tag, &task->tags) {
                if (!sv_eq(*tag, op->as.sv)) {
                    da_append(&tags, *tag);
                }
            }
            if (tags.count != task->tags.count) {
                da_free(task->tags);
                task->tags = tags;
                changed = true;
            }
        } break;
        default: UNREACHABLE("Ml_Op_Kind");
        }
    }
    return changed;
}

Ml_Op ml_op(Ml_Op_Kind kind, String_View src) {
    return (Ml_Op) { .kind = kind, .src = src };
}

Ml_Op ml_op_store_prop(Ml_Op op, Property prop) {
    op.as.prop = prop;
    return op;
}

Ml_Op ml_op_store_integer(Ml_Op op, long value) {
    op.as.integer = value;
    return op;
}

Ml_Op ml_op_store_sv(Ml_Op op, String_View value) {
    op.as.sv = value;
    return op;
}

Ml_Op ml_op_store_tags(Ml_Op op, Tags tags) {
    op.as.tags = tags;
    return op;
}
