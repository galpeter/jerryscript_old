/* Copyright 2014-2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * String built-in description
 */

#ifndef OBJECT_ID
# define OBJECT_ID(builtin_object_id)
#endif /* !OBJECT_ID */

#ifndef NUMBER_VALUE
# define NUMBER_VALUE(name, number_value, prop_writable, prop_enumerable, prop_configurable)
#endif /* !NUMBER_VALUE */

#ifndef OBJECT_VALUE
# define OBJECT_VALUE(name, obj_getter, prop_writable, prop_enumerable, prop_configurable)
#endif /* !OBJECT_VALUE */

#ifndef ROUTINE
# define ROUTINE(name, c_function_name, args_number, length_prop_value)
#endif /* !ROUTINE */

/* Object identifier */
OBJECT_ID (ECMA_BUILTIN_ID_STRING)

/* Number properties:
 *  (property name, number value, writable, enumerable, configurable) */

// 15.5.3
NUMBER_VALUE (ECMA_MAGIC_STRING_LENGTH,
              1,
              ECMA_PROPERTY_NOT_WRITABLE,
              ECMA_PROPERTY_NOT_ENUMERABLE,
              ECMA_PROPERTY_NOT_CONFIGURABLE)

/* Object properties:
 *  (property name, object pointer getter) */

// 15.7.3.1
OBJECT_VALUE (ECMA_MAGIC_STRING_PROTOTYPE,
              ecma_builtin_get (ECMA_BUILTIN_ID_STRING_PROTOTYPE),
              ECMA_PROPERTY_NOT_WRITABLE,
              ECMA_PROPERTY_NOT_ENUMERABLE,
              ECMA_PROPERTY_NOT_CONFIGURABLE)

/* Routine properties:
 *  (property name, C routine name, arguments number or NON_FIXED, value of the routine's length property) */
ROUTINE (ECMA_MAGIC_STRING_FROM_CHAR_CODE_UL, ecma_builtin_string_object_from_char_code, NON_FIXED, 1)

#undef OBJECT_ID
#undef SIMPLE_VALUE
#undef NUMBER_VALUE
#undef STRING_VALUE
#undef OBJECT_VALUE
#undef CP_UNIMPLEMENTED_VALUE
#undef ROUTINE
