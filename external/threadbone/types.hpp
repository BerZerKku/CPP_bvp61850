//////////////////////////////////////////////////////////////////////////////
// <@meta=.metablank>
//////////////////////////////////////////////////////////////////////////////
# ifndef SW_TYPES_INCLUDED_
#  define SW_TYPES_INCLUDED_

# include <iostream>
# include <string>


# include "threadbone.hpp"

# include "iec61850_common.h"

namespace swissknife {
namespace types      {

using namespace std;
namespace TB = threadbone;

// NOTE: модифицировать внимательно! Перечисляемые значения TChannelType должны
//  соответствовать индексам указателей на функции в векторах:
//   - s_get_server_da_value
//   - s_set_server_da_value
//   - s_is_mms_value_equals
//   - ... (список может быть расширен)
//  в файле tasks/iec61850.hpp
typedef enum {
    CH_TYPE_UNKNOWN = 0,
    CH_TYPE_BOOL,
    CH_TYPE_I8,
    CH_TYPE_U8,
    CH_TYPE_I16,
    CH_TYPE_U16,
    CH_TYPE_I32,
    CH_TYPE_U32,
    CH_TYPE_I64,
    CH_TYPE_U64,
    CH_TYPE_F32,
    CH_TYPE_F64,
    CH_TYPE_UTS, // unix timestamp in uS
    CH_TYPE_IEC61850_Q,
    CH_TYPE_IEC61850_T
} TChannelType;

// TODO: рассмотреть возможность использования std::variant вместо union
typedef union {
    int64_t   max;
    bool      b;
    int8_t    i8;
    uint8_t   u8;
    int16_t   i16;
    uint16_t  u16;
    int32_t   i32;
    uint32_t  u32;
    int64_t   i64;
    uint64_t  u64;
    float     f32;
    double    f64;
    int64_t   uts; // int - для удобства вычитания из меньшего большего
    Quality   iec61850_q;
    Timestamp iec61850_t;
} TChannelValue;

static string
channel_type_to_string(TChannelType ch_type) {
    switch (ch_type) {
        case CH_TYPE_UNKNOWN    : return "X";
        case CH_TYPE_BOOL       : return "BOOL";
        case CH_TYPE_I8         : return "I8";
        case CH_TYPE_U8         : return "U8";
        case CH_TYPE_I16        : return "I16";
        case CH_TYPE_U16        : return "U16";
        case CH_TYPE_I32        : return "I32";
        case CH_TYPE_U32        : return "U32";
        case CH_TYPE_I64        : return "I64";
        case CH_TYPE_U64        : return "U64";
        case CH_TYPE_F32        : return "F32";
        case CH_TYPE_F64        : return "F64";
        case CH_TYPE_UTS        : return "UTS";
        case CH_TYPE_IEC61850_Q : return "IEC61850_Q";
        case CH_TYPE_IEC61850_T : return "IEC61850_T";
        default                 : return "X";
    }
}

static TChannelType
channel_type_from_string(string const & str) {
    if ( str == "X" )          return CH_TYPE_UNKNOWN;
    if ( str == "BOOL" )       return CH_TYPE_BOOL;
    if ( str == "I8" )         return CH_TYPE_I8;
    if ( str == "U8" )         return CH_TYPE_U8;
    if ( str == "I16" )        return CH_TYPE_I16;
    if ( str == "U16" )        return CH_TYPE_U16;
    if ( str == "I32" )        return CH_TYPE_I32;
    if ( str == "U32" )        return CH_TYPE_U32;
    if ( str == "I64" )        return CH_TYPE_I64;
    if ( str == "U64" )        return CH_TYPE_U64;
    if ( str == "F32" )        return CH_TYPE_F32;
    if ( str == "F64" )        return CH_TYPE_F64;
    if ( str == "UTS" )        return CH_TYPE_UTS;
    if ( str == "IEC61850_Q" ) return CH_TYPE_IEC61850_Q;
    if ( str == "IEC61850_T" ) return CH_TYPE_IEC61850_T;
    return CH_TYPE_UNKNOWN;
}

struct TChannel {

    TChannelType
    type;

    TChannelValue
    value;

    // К-тор по умолчанию
    TChannel() : type(CH_TYPE_UNKNOWN) { value.max=0; }

    // NOTE: 'value.max = 0;' для очистки от мусора MSB, если канал использует
    // не полную ширину.
    TChannel(bool     v) : type(CH_TYPE_BOOL) { value.max = 0; value.b   = v; }
    TChannel(int8_t   v) : type(CH_TYPE_I8)   { value.max = 0; value.i8  = v; }
    TChannel(uint8_t  v) : type(CH_TYPE_U8)   { value.max = 0; value.u8  = v; }
    TChannel(int16_t  v) : type(CH_TYPE_I16)  { value.max = 0; value.i16 = v; }
    TChannel(uint16_t v) : type(CH_TYPE_U16)  { value.max = 0; value.u16 = v; }
    TChannel(int32_t  v) : type(CH_TYPE_I32)  { value.max = 0; value.i32 = v; }
    TChannel(uint32_t v) : type(CH_TYPE_U32)  { value.max = 0; value.u32 = v; }
    TChannel(int64_t  v) : type(CH_TYPE_I64)  { value.max = 0; value.i64 = v; }
    TChannel(uint64_t v) : type(CH_TYPE_U64)  { value.max = 0; value.u64 = v; }
    TChannel(float    v) : type(CH_TYPE_F32)  { value.max = 0; value.f32 = v; }
    TChannel(double   v) : type(CH_TYPE_F64)  { value.max = 0; value.f64 = v; }

    TChannel(Timestamp const & v)
      : type(CH_TYPE_IEC61850_T)
    {
        value.iec61850_t.val[0] = v.val[0];
        value.iec61850_t.val[1] = v.val[1];
        value.iec61850_t.val[2] = v.val[2];
        value.iec61850_t.val[3] = v.val[3];
        value.iec61850_t.val[4] = v.val[4];
        value.iec61850_t.val[5] = v.val[5];
        value.iec61850_t.val[6] = v.val[6];
        value.iec61850_t.val[7] = v.val[7];
    }

    TChannel(TChannelType t, uint16_t v ) {

        type      = CH_TYPE_UNKNOWN;
        value.max = 0;

        if ( t == CH_TYPE_IEC61850_Q ) {
            type  = CH_TYPE_IEC61850_Q;
            value.u16 = v;
        }
    }

    TChannel(TChannelType type) : type(type) { value.max=0; }

    TChannel(TChannelType type, TChannelValue value )
      : type(type),
        value(value)
    {}

    // NOTE: при вызове этого к.тора, использовать явное приведение типа,
    // например, TChannel((string const &)"X"), иначе вызывается другой к.тор
    TChannel(string const & str)
      : type(channel_type_from_string(str))
    {
        value.max=0;
    }

    // К-тор копирования
    TChannel(TChannel const & other)
     :  type(other.type),
        value(other.value)
    {}

    void
    operator=(TChannel const &other) {
        type=other.type;
        value=other.value;
    }

    bool
    operator==(TChannel const & other) const {
        return
            type == other.type &&
            value.max == other.value.max;
    }

    bool
    operator!=(TChannel const & other) const {
        return
            type != other.type ||
            value.max != other.value.max;
    }

    inline void
    set_value(TChannel const & other) {
        // NOTE: 'value.max = 0;' для очистки от мусора MSB, если канал
        // использует не полную ширину.
        value.max = 0;
        switch (type) {
            case CH_TYPE_BOOL:
                switch (other.type) {
                    case CH_TYPE_BOOL       : value = other.value; return;
                    case CH_TYPE_I8         : value.b = (bool) other.value.i8;  return;
                    case CH_TYPE_U8         : value.b = (bool) other.value.u8;  return;
                    case CH_TYPE_I16        : value.b = (bool) other.value.i16; return;
                    case CH_TYPE_U16        : value.b = (bool) other.value.u16; return;
                    case CH_TYPE_I32        : value.b = (bool) other.value.i32; return;
                    case CH_TYPE_U32        : value.b = (bool) other.value.u32; return;
                    case CH_TYPE_I64        : value.b = (bool) other.value.i64; return;
                    case CH_TYPE_U64        : value.b = (bool) other.value.u64; return;
                    case CH_TYPE_F32        : value.b = (bool) other.value.f32; return;
                    case CH_TYPE_F64        : value.b = (bool) other.value.f64; return;
                    case CH_TYPE_UNKNOWN    :
                    case CH_TYPE_UTS        :
                    case CH_TYPE_IEC61850_Q :
                    case CH_TYPE_IEC61850_T :
                    default                 : value = other.value; return;
                }
            case CH_TYPE_I8:
                switch (other.type) {
                    case CH_TYPE_BOOL       : value.i8 = (int8_t) other.value.b;   return;
                    case CH_TYPE_I8         : value = other.value; return;
                    case CH_TYPE_U8         : value.i8 = (int8_t) other.value.u8;  return;
                    case CH_TYPE_I16        : value.i8 = (int8_t) other.value.i16; return;
                    case CH_TYPE_U16        : value.i8 = (int8_t) other.value.u16; return;
                    case CH_TYPE_I32        : value.i8 = (int8_t) other.value.i32; return;
                    case CH_TYPE_U32        : value.i8 = (int8_t) other.value.u32; return;
                    case CH_TYPE_I64        : value.i8 = (int8_t) other.value.i64; return;
                    case CH_TYPE_U64        : value.i8 = (int8_t) other.value.u64; return;
                    case CH_TYPE_F32        : value.i8 = (int8_t) other.value.f32; return;
                    case CH_TYPE_F64        : value.i8 = (int8_t) other.value.f64; return;
                    case CH_TYPE_UNKNOWN    :
                    case CH_TYPE_UTS        :
                    case CH_TYPE_IEC61850_Q :
                    case CH_TYPE_IEC61850_T :
                    default                 : value = other.value; return;
                }
            case CH_TYPE_U8:
                switch (other.type) {
                    case CH_TYPE_BOOL       : value.u8 = (uint8_t) other.value.b;   return;
                    case CH_TYPE_I8         : value.u8 = (uint8_t) other.value.i8;  return;
                    case CH_TYPE_U8         : value = other.value; return;
                    case CH_TYPE_I16        : value.u8 = (uint8_t) other.value.i16; return;
                    case CH_TYPE_U16        : value.u8 = (uint8_t) other.value.u16; return;
                    case CH_TYPE_I32        : value.u8 = (uint8_t) other.value.i32; return;
                    case CH_TYPE_U32        : value.u8 = (uint8_t) other.value.u32; return;
                    case CH_TYPE_I64        : value.u8 = (uint8_t) other.value.i64; return;
                    case CH_TYPE_U64        : value.u8 = (uint8_t) other.value.u64; return;
                    case CH_TYPE_F32        : value.u8 = (uint8_t) other.value.f32; return;
                    case CH_TYPE_F64        : value.u8 = (uint8_t) other.value.f64; return;
                    case CH_TYPE_UNKNOWN    :
                    case CH_TYPE_UTS        :
                    case CH_TYPE_IEC61850_Q :
                    case CH_TYPE_IEC61850_T :
                    default                 : value = other.value; return;
                }
            case CH_TYPE_I16:
                switch (other.type) {
                    case CH_TYPE_BOOL       : value.i16 = (int16_t) other.value.b;   return;
                    case CH_TYPE_I8         : value.i16 = (int16_t) other.value.i8;  return;
                    case CH_TYPE_U8         : value.i16 = (int16_t) other.value.u8;  return;
                    case CH_TYPE_I16        : value = other.value; return;
                    case CH_TYPE_U16        : value.i16 = (int16_t) other.value.u16; return;
                    case CH_TYPE_I32        : value.i16 = (int16_t) other.value.i32; return;
                    case CH_TYPE_U32        : value.i16 = (int16_t) other.value.u32; return;
                    case CH_TYPE_I64        : value.i16 = (int16_t) other.value.i64; return;
                    case CH_TYPE_U64        : value.i16 = (int16_t) other.value.u64; return;
                    case CH_TYPE_F32        : value.i16 = (int16_t) other.value.f32; return;
                    case CH_TYPE_F64        : value.i16 = (int16_t) other.value.f64; return;
                    case CH_TYPE_UNKNOWN    :
                    case CH_TYPE_UTS        :
                    case CH_TYPE_IEC61850_Q :
                    case CH_TYPE_IEC61850_T :
                    default                 : value = other.value; return;
                }
            case CH_TYPE_U16:
                switch (other.type) {
                    case CH_TYPE_BOOL       : value.u16 = (uint16_t) other.value.b;   return;
                    case CH_TYPE_I8         : value.u16 = (uint16_t) other.value.i8;  return;
                    case CH_TYPE_U8         : value.u16 = (uint16_t) other.value.u8;  return;
                    case CH_TYPE_I16        : value.u16 = (uint16_t) other.value.i16; return;
                    case CH_TYPE_U16        : value = other.value; return;
                    case CH_TYPE_I32        : value.u16 = (uint16_t) other.value.i32; return;
                    case CH_TYPE_U32        : value.u16 = (uint16_t) other.value.u32; return;
                    case CH_TYPE_I64        : value.u16 = (uint16_t) other.value.i64; return;
                    case CH_TYPE_U64        : value.u16 = (uint16_t) other.value.u64; return;
                    case CH_TYPE_F32        : value.u16 = (uint16_t) other.value.f32; return;
                    case CH_TYPE_F64        : value.u16 = (uint16_t) other.value.f64; return;
                    case CH_TYPE_UNKNOWN    :
                    case CH_TYPE_UTS        :
                    case CH_TYPE_IEC61850_Q :
                    case CH_TYPE_IEC61850_T :
                    default                 : value = other.value; return;
                }
            case CH_TYPE_I32:
                switch (other.type) {
                    case CH_TYPE_BOOL       : value.i32 = (int32_t) other.value.b;   return;
                    case CH_TYPE_I8         : value.i32 = (int32_t) other.value.i8;  return;
                    case CH_TYPE_U8         : value.i32 = (int32_t) other.value.u8;  return;
                    case CH_TYPE_I16        : value.i32 = (int32_t) other.value.i16; return;
                    case CH_TYPE_U16        : value.i32 = (int32_t) other.value.u16; return;
                    case CH_TYPE_I32        : value = other.value; return;
                    case CH_TYPE_U32        : value.i32 = (int32_t) other.value.u32; return;
                    case CH_TYPE_I64        : value.i32 = (int32_t) other.value.i64; return;
                    case CH_TYPE_U64        : value.i32 = (int32_t) other.value.u64; return;
                    case CH_TYPE_F32        : value.i32 = (int32_t) other.value.f32; return;
                    case CH_TYPE_F64        : value.i32 = (int32_t) other.value.f64; return;
                    case CH_TYPE_UNKNOWN    :
                    case CH_TYPE_UTS        :
                    case CH_TYPE_IEC61850_Q :
                    case CH_TYPE_IEC61850_T :
                    default                 : value = other.value; return;
                }
            case CH_TYPE_U32:
                switch (other.type) {
                    case CH_TYPE_BOOL       : value.u32 = (uint32_t) other.value.b;   return;
                    case CH_TYPE_I8         : value.u32 = (uint32_t) other.value.i8;  return;
                    case CH_TYPE_U8         : value.u32 = (uint32_t) other.value.u8;  return;
                    case CH_TYPE_I16        : value.u32 = (uint32_t) other.value.i16; return;
                    case CH_TYPE_U16        : value.u32 = (uint32_t) other.value.u16; return;
                    case CH_TYPE_I32        : value.u32 = (uint32_t) other.value.i32; return;
                    case CH_TYPE_U32        : value = other.value; return;
                    case CH_TYPE_I64        : value.u32 = (uint32_t) other.value.i64; return;
                    case CH_TYPE_U64        : value.u32 = (uint32_t) other.value.u64; return;
                    case CH_TYPE_F32        : value.u32 = (uint32_t) other.value.f32; return;
                    case CH_TYPE_F64        : value.u32 = (uint32_t) other.value.f64; return;
                    case CH_TYPE_UNKNOWN    :
                    case CH_TYPE_UTS        :
                    case CH_TYPE_IEC61850_Q :
                    case CH_TYPE_IEC61850_T :
                    default                 : value = other.value; return;
                }
            case CH_TYPE_I64:
                switch (other.type) {
                    case CH_TYPE_BOOL       : value.i64 = (int64_t) other.value.b;   return;
                    case CH_TYPE_I8         : value.i64 = (int64_t) other.value.i8;  return;
                    case CH_TYPE_U8         : value.i64 = (int64_t) other.value.u8;  return;
                    case CH_TYPE_I16        : value.i64 = (int64_t) other.value.i16; return;
                    case CH_TYPE_U16        : value.i64 = (int64_t) other.value.u16; return;
                    case CH_TYPE_I32        : value.i64 = (int64_t) other.value.i32; return;
                    case CH_TYPE_U32        : value.i64 = (int64_t) other.value.u32; return;
                    case CH_TYPE_I64        : value = other.value; return;
                    case CH_TYPE_U64        : value.i64 = (int64_t) other.value.u64; return;
                    case CH_TYPE_F32        : value.i64 = (int64_t) other.value.f32; return;
                    case CH_TYPE_F64        : value.i64 = (int64_t) other.value.f64; return;
                    case CH_TYPE_UNKNOWN    :
                    case CH_TYPE_UTS        :
                    case CH_TYPE_IEC61850_Q :
                    case CH_TYPE_IEC61850_T :
                    default                 : value = other.value; return;
                }
            case CH_TYPE_U64:
                switch (other.type) {
                    case CH_TYPE_BOOL       : value.u64 = (uint64_t) other.value.b;   return;
                    case CH_TYPE_I8         : value.u64 = (uint64_t) other.value.i8;  return;
                    case CH_TYPE_U8         : value.u64 = (uint64_t) other.value.u8;  return;
                    case CH_TYPE_I16        : value.u64 = (uint64_t) other.value.i16; return;
                    case CH_TYPE_U16        : value.u64 = (uint64_t) other.value.u16; return;
                    case CH_TYPE_I32        : value.u64 = (uint64_t) other.value.i32; return;
                    case CH_TYPE_U32        : value.u64 = (uint64_t) other.value.u32; return;
                    case CH_TYPE_I64        : value.u64 = (uint64_t) other.value.i64; return;
                    case CH_TYPE_U64        : value = other.value; return;
                    case CH_TYPE_F32        : value.u64 = (uint64_t) other.value.f32; return;
                    case CH_TYPE_F64        : value.u64 = (uint64_t) other.value.f64; return;
                    case CH_TYPE_UNKNOWN    :
                    case CH_TYPE_UTS        :
                    case CH_TYPE_IEC61850_Q :
                    case CH_TYPE_IEC61850_T :
                    default                 : value = other.value; return;
                }
            case CH_TYPE_F32:
                switch (other.type) {
                    case CH_TYPE_BOOL       : value.f32 = (float) other.value.b;   return;
                    case CH_TYPE_I8         : value.f32 = (float) other.value.i8;  return;
                    case CH_TYPE_U8         : value.f32 = (float) other.value.u8;  return;
                    case CH_TYPE_I16        : value.f32 = (float) other.value.i16; return;
                    case CH_TYPE_U16        : value.f32 = (float) other.value.u16; return;
                    case CH_TYPE_I32        : value.f32 = (float) other.value.i32; return;
                    case CH_TYPE_U32        : value.f32 = (float) other.value.u32; return;
                    case CH_TYPE_I64        : value.f32 = (float) other.value.i64; return;
                    case CH_TYPE_U64        : value.f32 = (float) other.value.u64; return;
                    case CH_TYPE_F32        : value = other.value; return;
                    case CH_TYPE_F64        : value.f32 = (float) other.value.f64; return;
                    case CH_TYPE_UNKNOWN    :
                    case CH_TYPE_UTS        :
                    case CH_TYPE_IEC61850_Q :
                    case CH_TYPE_IEC61850_T :
                    default                 : value = other.value; return;
                }
            case CH_TYPE_F64:
                switch (other.type) {
                    case CH_TYPE_BOOL       : value.f64 = (double) other.value.b;   return;
                    case CH_TYPE_I8         : value.f64 = (double) other.value.i8;  return;
                    case CH_TYPE_U8         : value.f64 = (double) other.value.u8;  return;
                    case CH_TYPE_I16        : value.f64 = (double) other.value.i16; return;
                    case CH_TYPE_U16        : value.f64 = (double) other.value.u16; return;
                    case CH_TYPE_I32        : value.f64 = (double) other.value.i32; return;
                    case CH_TYPE_U32        : value.f64 = (double) other.value.u32; return;
                    case CH_TYPE_I64        : value.f64 = (double) other.value.i64; return;
                    case CH_TYPE_U64        : value.f64 = (double) other.value.u64; return;
                    case CH_TYPE_F32        : value.f64 = (double) other.value.f32; return;
                    case CH_TYPE_F64        : value = other.value; return;
                    case CH_TYPE_UNKNOWN    :
                    case CH_TYPE_UTS        :
                    case CH_TYPE_IEC61850_Q :
                    case CH_TYPE_IEC61850_T :
                    default                 : value = other.value; return;
                }
            case CH_TYPE_UNKNOWN    :
            case CH_TYPE_UTS        :
            case CH_TYPE_IEC61850_Q :
            case CH_TYPE_IEC61850_T :
            default                 : value = other.value; return;
        }
    }

    // set value of base type
    inline void
    set_btype_value(auto v) {
        // NOTE: 'value.max = 0;' для очистки от мусора MSB, если канал
        // использует не полную ширину.
        value.max = 0;
        switch(type) {
            case CH_TYPE_BOOL       : value.b   = (bool)     v; return;
            case CH_TYPE_I8         : value.i8  = (int8_t)   v; return;
            case CH_TYPE_U8         : value.u8  = (uint8_t)  v; return;
            case CH_TYPE_I16        : value.i16 = (int16_t)  v; return;
            case CH_TYPE_U16        : value.u16 = (uint16_t) v; return;
            case CH_TYPE_I32        : value.i32 = (int32_t)  v; return;
            case CH_TYPE_U32        : value.u32 = (uint32_t) v; return;
            case CH_TYPE_I64        : value.i64 = (int64_t)  v; return;
            case CH_TYPE_U64        : value.u64 = (uint64_t) v; return;
            case CH_TYPE_F32        : value.f32 = (float)    v; return;
            case CH_TYPE_F64        : value.f64 = (double)   v; return;
            case CH_TYPE_UNKNOWN    :
            case CH_TYPE_UTS        :
            case CH_TYPE_IEC61850_Q :
            case CH_TYPE_IEC61850_T :
            default                 : return;
        }
    }

    // get value of base type
    inline void
    get_btype_value(auto * var) {
        auto
        tmp = *var;
        tmp = 0;
        switch(type) {
            case CH_TYPE_BOOL       : tmp = (decltype(tmp)) value.b;   break;
            case CH_TYPE_I8         : tmp = (decltype(tmp)) value.i8;  break;
            case CH_TYPE_U8         : tmp = (decltype(tmp)) value.u8;  break;
            case CH_TYPE_I16        : tmp = (decltype(tmp)) value.i16; break;
            case CH_TYPE_U16        : tmp = (decltype(tmp)) value.u16; break;
            case CH_TYPE_I32        : tmp = (decltype(tmp)) value.i32; break;
            case CH_TYPE_U32        : tmp = (decltype(tmp)) value.u32; break;
            case CH_TYPE_I64        : tmp = (decltype(tmp)) value.i64; break;
            case CH_TYPE_U64        : tmp = (decltype(tmp)) value.u64; break;
            case CH_TYPE_F32        : tmp = (decltype(tmp)) value.f32; break;
            case CH_TYPE_F64        : tmp = (decltype(tmp)) value.f64; break;
            case CH_TYPE_UNKNOWN    :
            case CH_TYPE_UTS        :
            case CH_TYPE_IEC61850_Q :
            case CH_TYPE_IEC61850_T :
            default                 : break;
        }
        *var = tmp;
    }

};

// TODO: устарела. Заменить на 'channel_type_to_string'
static void
print_channel_type(TChannel const * p_ch) {
    switch (p_ch->type) {
        case CH_TYPE_UNKNOWN    : cout << "X";    return;
        case CH_TYPE_BOOL       : cout << "BOOL"; return;
        case CH_TYPE_I8         : cout << "I8";   return;
        case CH_TYPE_U8         : cout << "U8";   return;
        case CH_TYPE_I16        : cout << "I16";  return;
        case CH_TYPE_U16        : cout << "U16";  return;
        case CH_TYPE_I32        : cout << "I32";  return;
        case CH_TYPE_U32        : cout << "U32";  return;
        case CH_TYPE_I64        : cout << "I64";  return;
        case CH_TYPE_U64        : cout << "U64";  return;
        case CH_TYPE_F32        : cout << "F32";  return;
        case CH_TYPE_F64        : cout << "F64";  return;
        case CH_TYPE_UTS        : cout << "UTS";  return;
        case CH_TYPE_IEC61850_Q : cout << "Q";    return;
        case CH_TYPE_IEC61850_T : cout << "T";    return;
    }
}

static void
print_channel_value(TChannel const * p_ch) {
    switch (p_ch->type) {
        case CH_TYPE_UNKNOWN    : cout << p_ch->value.max;        return;
        case CH_TYPE_BOOL       : cout << p_ch->value.b;          return;
        case CH_TYPE_I8         : cout << p_ch->value.i8;         return;
        case CH_TYPE_U8         : cout << p_ch->value.u8;         return;
        case CH_TYPE_I16        : cout << p_ch->value.i16;        return;
        case CH_TYPE_U16        : cout << p_ch->value.u16;        return;
        case CH_TYPE_I32        : cout << p_ch->value.i32;        return;
        case CH_TYPE_U32        : cout << p_ch->value.u32;        return;
        case CH_TYPE_I64        : cout << p_ch->value.i64;        return;
        case CH_TYPE_U64        : cout << p_ch->value.u64;        return;
        case CH_TYPE_F32        : cout << p_ch->value.f32;        return;
        case CH_TYPE_F64        : cout << p_ch->value.f64;        return;
        case CH_TYPE_UTS        : cout << p_ch->value.uts;        return;
        case CH_TYPE_IEC61850_Q : cout << p_ch->value.iec61850_q; return;
        case CH_TYPE_IEC61850_T : cout << p_ch->value.max;        return;
        // TODO:                  cout << p_ch->value.iec61850_t; return;
    }
}

//typedef TB::Port<TChannel> TPort;

}                                                          //    types
}                                                          // swissknife
# endif                                                    // SW_TYPES_INCLUDED_

/*
 * NOTE:
 * long double - 16 байт
 * */
