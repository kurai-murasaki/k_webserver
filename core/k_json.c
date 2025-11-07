#include "k_json.h"
#include <math.h>



K_jsonValue* k_json_parse(const char *json_str, size_t json_str_len){

}


void k_json_free(K_jsonValue* value){
    if(!value) return;
    switch(value->type){
        case K_JSON_STRING:
            free(value->string_value);
            break;
        case K_JSON_ARRAY:
            for(size_t i = 0; i < value->array.count; i++){
                k_json_free(value->array.items[i]);
            }
            free(value->array.items);
            break;
        case K_JSON_OBJECT:
            for(size_t i = 0; i < value->object.count; i++){
                free(value->object.keys[i]);
                k_json_free(value->object.values[i]);
            }
            free(value->object.keys);
            free(value->object.values);
            break;
        default:
            break;
    }
    free(value);
}


K_jsonValue* k_json_get_object(K_jsonValue* obj, const char* key){

}


int k_json_bool(K_jsonValue* value){
    if(!value) return -1;
    if(value->type != K_JSON_BOOL) return -1;
    return value->bool_value;
}

double k_json_float(K_jsonValue* value){
    if(!value) return NAN;
    if(value->type != K_JSON_FLOAT) return NAN;
    return value->float_value;
}

int k_json_int(K_jsonValue* value){
    if(!value) return NAN;
    if(value->type != K_JSON_INT) return NAN;
    return value->int_value;
}

const char* k_json_string(K_jsonValue* value){
    if(!value) return NULL;
    if(value->type != K_JSON_STRING) return NULL;
    return value->string_value;
}