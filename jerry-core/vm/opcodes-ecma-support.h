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

#ifndef OPCODES_ECMA_SUPPORT_H
#define OPCODES_ECMA_SUPPORT_H

#include "ecma-alloc.h"
#include "ecma-array-object.h"
#include "ecma-comparison.h"
#include "ecma-conversion.h"
#include "ecma-exceptions.h"
#include "ecma-function-object.h"
#include "ecma-gc.h"
#include "ecma-helpers.h"
#include "ecma-lex-env.h"
#include "ecma-number-arithmetic.h"
#include "ecma-objects.h"
#include "ecma-objects-general.h"
#include "ecma-reference.h"
#include "ecma-try-catch-macro.h"
#include "serializer.h"

bool is_reg_variable (int_data_t *int_data, idx_t var_idx);
ecma_completion_value_t get_variable_value (int_data_t *, idx_t, bool);
ecma_completion_value_t set_variable_value (int_data_t *, opcode_counter_t, idx_t, ecma_value_t);
ecma_completion_value_t fill_varg_list (int_data_t *int_data,
                                        ecma_length_t args_number,
                                        ecma_value_t args_values[],
                                        ecma_length_t *out_arg_number_p);
void fill_params_list (int_data_t *int_data,
                       ecma_length_t params_number,
                       ecma_string_t* params_names[]);
#endif /* OPCODES_ECMA_SUPPORT_H */
