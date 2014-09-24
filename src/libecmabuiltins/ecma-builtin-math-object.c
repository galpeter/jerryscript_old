/* Copyright 2014 Samsung Electronics Co., Ltd.
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

#include "ecma-alloc.h"
#include "ecma-builtins.h"
#include "ecma-conversion.h"
#include "ecma-exceptions.h"
#include "ecma-gc.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-number-arithmetic.h"
#include "ecma-objects.h"
#include "ecma-objects-general.h"
#include "ecma-try-catch-macro.h"
#include "globals.h"

#define ECMA_BUILTINS_INTERNAL
#include "ecma-builtins-internal.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmabuiltins
 * @{
 *
 * \addtogroup object ECMA Object object built-in
 * @{
 */

#if defined (CONFIG_ECMA_NUMBER_FLOAT32)
const ecma_number_t ecma_builtin_math_object_relative_eps = 1.0e-10f;
#elif defined (CONFIG_ECMA_NUMBER_FLOAT64)
const ecma_number_t ecma_builtin_math_object_relative_eps = 1.0e-16;
#else /* !CONFIG_ECMA_NUMBER_FLOAT32 && !CONFIG_ECMA_NUMBER_FLOAT64 */
# error "!CONFIG_ECMA_NUMBER_FLOAT32 && !CONFIG_ECMA_NUMBER_FLOAT64"
#endif /* !CONFIG_ECMA_NUMBER_FLOAT32 && !CONFIG_ECMA_NUMBER_FLOAT64 */

/**
 * List of the Math object built-in value properties in format 'macro (name, value)'.
 */
#define ECMA_BUILTIN_MATH_OBJECT_VALUES_PROPERTY_LIST(macro) \
  macro (ECMA_MAGIC_STRING_E_U,       2.7182818284590452354) \
  macro (ECMA_MAGIC_STRING_LN10_U,    2.302585092994046) \
  macro (ECMA_MAGIC_STRING_LN2_U,     0.6931471805599453) \
  macro (ECMA_MAGIC_STRING_LOG2E_U,   1.4426950408889634) \
  macro (ECMA_MAGIC_STRING_LOG10E_U,  0.4342944819032518) \
  macro (ECMA_MAGIC_STRING_PI_U,      3.1415926535897932) \
  macro (ECMA_MAGIC_STRING_SQRT1_2_U, 0.7071067811865476) \
  macro (ECMA_MAGIC_STRING_SQRT2_U,   1.4142135623730951)

/**
 * List of the Math object built-in routine properties in format
 * 'macro (name, C function name, arguments number of the routine, length value of the routine)'.
 */
#define ECMA_BUILTIN_MATH_OBJECT_ROUTINES_PROPERTY_LIST(macro) \
  macro (ECMA_MAGIC_STRING_ABS, \
         ecma_builtin_math_object_abs, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_ACOS, \
         ecma_builtin_math_object_acos, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_ASIN, \
         ecma_builtin_math_object_asin, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_ATAN, \
         ecma_builtin_math_object_atan, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_ATAN2, \
         ecma_builtin_math_object_atan2, \
         2, \
         2) \
  macro (ECMA_MAGIC_STRING_CEIL, \
         ecma_builtin_math_object_ceil, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_COS, \
         ecma_builtin_math_object_cos, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_EXP, \
         ecma_builtin_math_object_exp, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_FLOOR, \
         ecma_builtin_math_object_floor, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_LOG, \
         ecma_builtin_math_object_log, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_MAX, \
         ecma_builtin_math_object_max, \
         NON_FIXED, \
         2) \
  macro (ECMA_MAGIC_STRING_MIN, \
         ecma_builtin_math_object_min, \
         NON_FIXED, \
         2) \
  macro (ECMA_MAGIC_STRING_POW, \
         ecma_builtin_math_object_pow, \
         2, \
         2) \
  macro (ECMA_MAGIC_STRING_RANDOM, \
         ecma_builtin_math_object_random, \
         0, \
         0) \
  macro (ECMA_MAGIC_STRING_ROUND, \
         ecma_builtin_math_object_round, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_SIN, \
         ecma_builtin_math_object_sin, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_SQRT, \
         ecma_builtin_math_object_sqrt, \
         1, \
         1) \
  macro (ECMA_MAGIC_STRING_TAN, \
         ecma_builtin_math_object_tan, \
         1, \
         1)

/**
 * List of the Math object's built-in property names
 */
static const ecma_magic_string_id_t ecma_builtin_math_property_names[] =
{
#define VALUE_PROP_LIST(name, value) name,
#define ROUTINE_PROP_LIST(name, c_function_name, args_number, length) name,
  ECMA_BUILTIN_MATH_OBJECT_VALUES_PROPERTY_LIST (VALUE_PROP_LIST)
  ECMA_BUILTIN_MATH_OBJECT_ROUTINES_PROPERTY_LIST (ROUTINE_PROP_LIST)
#undef VALUE_PROP_LIST
#undef ROUTINE_PROP_LIST
};

/**
 * Number of the Math object's built-in properties
 */
const ecma_length_t ecma_builtin_math_property_number = (sizeof (ecma_builtin_math_property_names) /
                                                         sizeof (ecma_magic_string_id_t));
JERRY_STATIC_ASSERT (sizeof (ecma_builtin_math_property_names) > sizeof (void*));

/**
 * The Math object's 'abs' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_abs (ecma_value_t arg) /**< routine's argument */
{
  ecma_completion_value_t ret_value;

  ECMA_TRY_CATCH (arg_num_value,
                  ecma_op_to_number (arg),
                  ret_value);

  ecma_number_t *num_p = ecma_alloc_number ();

  const ecma_number_t arg_num = *(ecma_number_t*) ECMA_GET_POINTER (arg_num_value.u.value.value);
  
  if (ecma_number_is_nan (arg_num)
      || !ecma_number_is_negative (arg_num))
  {
    *num_p = arg_num;
  }
  else
  {
    *num_p = ecma_number_negate (arg_num);
  }

  ret_value = ecma_make_normal_completion_value (ecma_make_number_value (num_p));

  ECMA_FINALIZE (arg_num_value);

  return ret_value;
} /* ecma_builtin_math_object_abs */

/**
 * The Math object's 'acos' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.2
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_acos (ecma_value_t arg) /**< routine's argument */
{
  JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (arg);
} /* ecma_builtin_math_object_acos */

/**
 * The Math object's 'asin' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.3
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_asin (ecma_value_t arg) /**< routine's argument */
{
  JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (arg);
} /* ecma_builtin_math_object_asin */

/**
 * The Math object's 'atan' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.4
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_atan (ecma_value_t arg) /**< routine's argument */
{
  JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (arg);
} /* ecma_builtin_math_object_atan */

/**
 * The Math object's 'atan2' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.5
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_atan2 (ecma_value_t arg1, /**< first routine's argument */
                                ecma_value_t arg2) /**< second routine's argument */
{
  JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (arg1, arg2);
} /* ecma_builtin_math_object_atan2 */

/**
 * The Math object's 'ceil' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.6
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_ceil (ecma_value_t arg) /**< routine's argument */
{
  JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (arg);
} /* ecma_builtin_math_object_ceil */

/**
 * The Math object's 'cos' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.7
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_cos (ecma_value_t arg) /**< routine's argument */
{
  JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (arg);
} /* ecma_builtin_math_object_cos */

/**
 * The Math object's 'exp' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.8
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_exp (ecma_value_t arg) /**< routine's argument */
{
  ecma_completion_value_t ret_value;

  ECMA_TRY_CATCH (arg_num_value,
                  ecma_op_to_number (arg),
                  ret_value);

  ecma_number_t *num_p = ecma_alloc_number ();

  const ecma_number_t arg_num = *(ecma_number_t*) ECMA_GET_POINTER (arg_num_value.u.value.value);
  
  if (ecma_number_is_nan (arg_num))
  {
    *num_p = arg_num;
  }
  else if (ecma_number_is_zero (arg_num))
  {
    *num_p = ECMA_NUMBER_ONE;
  }
  else if (ecma_number_is_infinity (arg_num))
  {
    if (ecma_number_is_negative (arg_num))
    {
      *num_p = ECMA_NUMBER_ZERO;
    }
    else
    {
      *num_p = arg_num;
    }
  }
  else
  {
    bool invert = false;
    ecma_number_t pow_e;

    if (ecma_number_is_negative (arg_num))
    {
      invert = true;
      pow_e = ecma_number_negate (arg_num);
    }
    else
    {
      pow_e = arg_num;
    }

    /* Taylor series of e^x is 1 + x/1! + x^2/2! + x^3/3! + ... + x^n/n! + ... */

    ecma_number_t sum = ECMA_NUMBER_ONE;
    ecma_number_t next_addendum = ecma_op_number_divide (pow_e, ECMA_NUMBER_ONE);
    ecma_number_t next_factorial_factor = ECMA_NUMBER_ONE;

    ecma_number_t diff = ecma_number_make_infinity (false);

    while (ecma_op_number_divide (diff, sum) > ecma_builtin_math_object_relative_eps)
    {
      ecma_number_t next_sum = ecma_op_number_add (sum, next_addendum);

      next_factorial_factor = ecma_op_number_add (next_factorial_factor, ECMA_NUMBER_ONE);
      next_addendum = ecma_op_number_multiply (next_addendum, pow_e);
      next_addendum = ecma_op_number_divide (next_addendum, next_factorial_factor);

      diff = ecma_op_number_substract (sum, next_sum);
      if (diff < 0)
      {
        diff = ecma_number_negate (diff);
      }

      sum = next_sum;
    }

    if (invert)
    {
      sum = ecma_op_number_divide (ECMA_NUMBER_ONE, sum);
    }

    *num_p = sum;
  }

  ret_value = ecma_make_normal_completion_value (ecma_make_number_value (num_p));

  ECMA_FINALIZE (arg_num_value);

  return ret_value;
} /* ecma_builtin_math_object_exp */

/**
 * The Math object's 'floor' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.9
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_floor (ecma_value_t arg) /**< routine's argument */
{
  JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (arg);
} /* ecma_builtin_math_object_floor */

/**
 * The Math object's 'log' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.10
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_log (ecma_value_t arg) /**< routine's argument */
{
  JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (arg);
} /* ecma_builtin_math_object_log */

/**
 * The Math object's 'max' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.11
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_max (ecma_value_t args[], /**< arguments list */
                              ecma_length_t args_number) /**< number of arguments */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  ecma_number_t ret_num = ecma_number_make_infinity (true);

  bool is_just_convert = false;

  for (ecma_length_t arg_index = 0;
       arg_index < args_number;
       arg_index++)
  {
    ECMA_TRY_CATCH (arg_num_value,
                    ecma_op_to_number (args[arg_index]),
                    ret_value);

    if (!is_just_convert)
    {
      ecma_number_t arg_num = *(ecma_number_t*) ECMA_GET_POINTER (arg_num_value.u.value.value);

      if (unlikely (ecma_number_is_nan (arg_num)))
      {
        ret_num = arg_num;
        is_just_convert = true;
      }
      else if (ecma_number_is_zero (arg_num) /* both numbers are zeroes */
               && ecma_number_is_zero (ret_num))
      {
        if (!ecma_number_is_negative (arg_num))
        {
          ret_num = arg_num;
        }
      }
      else if (ecma_number_is_infinity (arg_num))
      {
        if (!ecma_number_is_negative (arg_num))
        {
          ret_num = arg_num;
          is_just_convert = true;
        }
      }
      else if (ecma_number_is_infinity (ret_num)) /* ret_num is negative infinity */
      {
        JERRY_ASSERT (ecma_number_is_negative (ret_num));

        ret_num = arg_num;
      }
      else
      {
        JERRY_ASSERT (!ecma_number_is_nan (arg_num)
                      && !ecma_number_is_infinity (arg_num));
        JERRY_ASSERT (!ecma_number_is_nan (ret_num)
                      && !ecma_number_is_infinity (ret_num));

        if (arg_num > ret_num)
        {
          ret_num = arg_num;
        }
      }
    }

    ECMA_FINALIZE (arg_num_value);

    if (ecma_is_completion_value_throw (ret_value))
    {
      return ret_value;
    }

    JERRY_ASSERT (ecma_is_completion_value_empty (ret_value));
  }

  JERRY_ASSERT (ecma_is_completion_value_empty (ret_value));

  ecma_number_t *num_p = ecma_alloc_number ();
  *num_p = ret_num;

  return ecma_make_normal_completion_value (ecma_make_number_value (num_p));
} /* ecma_builtin_math_object_max */

/**
 * The Math object's 'min' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.12
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_min (ecma_value_t args[], /**< arguments list */
                              ecma_length_t args_number) /**< number of arguments */
{
  ecma_completion_value_t ret_value = ecma_make_empty_completion_value ();

  ecma_number_t ret_num = ecma_number_make_infinity (false);

  bool is_just_convert = false;

  for (ecma_length_t arg_index = 0;
       arg_index < args_number;
       arg_index++)
  {
    ECMA_TRY_CATCH (arg_num_value,
                    ecma_op_to_number (args[arg_index]),
                    ret_value);

    if (!is_just_convert)
    {
      ecma_number_t arg_num = *(ecma_number_t*) ECMA_GET_POINTER (arg_num_value.u.value.value);

      if (unlikely (ecma_number_is_nan (arg_num)))
      {
        ret_num = arg_num;
        is_just_convert = true;
      }
      else if (ecma_number_is_zero (arg_num) /* both numbers are zeroes */
               && ecma_number_is_zero (ret_num))
      {
        if (ecma_number_is_negative (arg_num))
        {
          ret_num = arg_num;
        }
      }
      else if (ecma_number_is_infinity (arg_num))
      {
        if (ecma_number_is_negative (arg_num))
        {
          ret_num = arg_num;
          is_just_convert = true;
        }
      }
      else if (ecma_number_is_infinity (ret_num)) /* ret_num is positive infinity */
      {
        JERRY_ASSERT (!ecma_number_is_negative (ret_num));

        ret_num = arg_num;
      }
      else
      {
        JERRY_ASSERT (!ecma_number_is_nan (arg_num)
                      && !ecma_number_is_infinity (arg_num));
        JERRY_ASSERT (!ecma_number_is_nan (ret_num)
                      && !ecma_number_is_infinity (ret_num));

        if (arg_num < ret_num)
        {
          ret_num = arg_num;
        }
      }
    }

    ECMA_FINALIZE (arg_num_value);

    if (ecma_is_completion_value_throw (ret_value))
    {
      return ret_value;
    }

    JERRY_ASSERT (ecma_is_completion_value_empty (ret_value));
  }

  JERRY_ASSERT (ecma_is_completion_value_empty (ret_value));

  ecma_number_t *num_p = ecma_alloc_number ();
  *num_p = ret_num;

  return ecma_make_normal_completion_value (ecma_make_number_value (num_p));
} /* ecma_builtin_math_object_min */

/**
 * The Math object's 'pow' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.13
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_pow (ecma_value_t arg1, /**< first routine's argument */
                              ecma_value_t arg2) /**< second routine's argument */
{
  JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (arg1, arg2);
} /* ecma_builtin_math_object_pow */

/**
 * The Math object's 'random' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.14
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_random (void)
{
  /* Implementation of George Marsaglia's XorShift random number generator */
  TODO (/* Check for license issues */);

  static uint32_t word1 = 1455997910;
  static uint32_t word2 = 1999515274;
  static uint32_t word3 = 1234451287;
  static uint32_t word4 = 1949149569;

  uint32_t intermediate = word1 ^ (word1 << 11);
  intermediate ^= intermediate >> 8;

  word1 = word2;
  word2 = word3;
  word3 = word4;

  word4 ^= word4 >> 19;
  word4 ^= intermediate;

  const uint32_t max_uint32 = (uint32_t) -1;
  ecma_number_t rand = (ecma_number_t) word4;
  rand /= (ecma_number_t) max_uint32;
  rand *= (ecma_number_t) (max_uint32 - 1) / (ecma_number_t) max_uint32;

  ecma_number_t *rand_p = ecma_alloc_number ();
  *rand_p = rand;

  return ecma_make_normal_completion_value (ecma_make_number_value (rand_p));
} /* ecma_builtin_math_object_random */

/**
 * The Math object's 'round' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.15
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_round (ecma_value_t arg) /**< routine's argument */
{
  ecma_completion_value_t ret_value;

  ECMA_TRY_CATCH (arg_num_value,
                  ecma_op_to_number (arg),
                  ret_value);

  ecma_number_t *num_p = ecma_alloc_number ();

  const ecma_number_t arg_num = *(ecma_number_t*) ECMA_GET_POINTER (arg_num_value.u.value.value);
  
  if (ecma_number_is_nan (arg_num)
      || ecma_number_is_zero (arg_num)
      || ecma_number_is_infinity (arg_num))
  {
    *num_p = arg_num;
  }
  else if (ecma_number_is_negative (arg_num)
           && arg_num >= -0.5f)
  {
    *num_p = ecma_number_negate (0.0f);
  }
  else
  {
    const ecma_number_t up_half = arg_num + 0.5f;
    const ecma_number_t down_half = arg_num - 0.5f;
    const ecma_number_t up_rounded = up_half - ecma_op_number_remainder (up_half, 1);
    const ecma_number_t down_rounded = down_half - ecma_op_number_remainder (down_half, 1);

    if (up_rounded - arg_num <= arg_num - down_rounded)
    {
      *num_p = up_rounded;
    }
    else
    {
      *num_p = down_rounded;
    }
  }

  ret_value = ecma_make_normal_completion_value (ecma_make_number_value (num_p));

  ECMA_FINALIZE (arg_num_value);

  return ret_value;
} /* ecma_builtin_math_object_round */

/**
 * The Math object's 'sin' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.16
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_sin (ecma_value_t arg) /**< routine's argument */
{
  JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (arg);
} /* ecma_builtin_math_object_sin */

/**
 * The Math object's 'sqrt' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.17
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_sqrt (ecma_value_t arg) /**< routine's argument */
{
  ecma_completion_value_t ret_value;

  ECMA_TRY_CATCH (arg_num_value,
                  ecma_op_to_number (arg),
                  ret_value);

  const ecma_number_t arg_num = *(ecma_number_t*) ECMA_GET_POINTER (arg_num_value.u.value.value);
  ecma_number_t ret_num;

  if (ecma_number_is_nan (arg_num)
      || (!ecma_number_is_zero (arg_num)
          && ecma_number_is_negative (arg_num)))
  {
    ret_num = ecma_number_make_nan ();
  }
  else if (ecma_number_is_zero (arg_num))
  {
    ret_num = arg_num;
  }
  else if (ecma_number_is_infinity (arg_num))
  {
    JERRY_ASSERT (!ecma_number_is_negative (arg_num));

    ret_num = arg_num;
  }
  else
  {
    /* Newton's method */

    ecma_number_t x = ECMA_NUMBER_ONE;
    ecma_number_t diff = ecma_number_make_infinity (false);

    while (ecma_op_number_divide (diff, x) > ecma_builtin_math_object_relative_eps)
    {
      ecma_number_t x_next = ecma_op_number_multiply (ECMA_NUMBER_HALF,
                                                      (ecma_op_number_add (x,
                                                                           ecma_op_number_divide (arg_num, x))));

      diff = ecma_op_number_substract (x, x_next);
      if (diff < 0)
      {
        diff = ecma_number_negate (diff);
      }

      x = x_next;
    }

    ret_num = x;
  }

  ecma_number_t *num_p = ecma_alloc_number ();
  *num_p = ret_num;

  ret_value = ecma_make_normal_completion_value (ecma_make_number_value (num_p));

  ECMA_FINALIZE (arg_num_value);

  return ret_value;
} /* ecma_builtin_math_object_sqrt */

/**
 * The Math object's 'tan' routine
 *
 * See also:
 *          ECMA-262 v5, 15.8.2.18
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static ecma_completion_value_t
ecma_builtin_math_object_tan (ecma_value_t arg) /**< routine's argument */
{
  JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (arg);
} /* ecma_builtin_math_object_tan */

/**
 * If the property's name is one of built-in properties of the Math object
 * that is not instantiated yet, instantiate the property and
 * return pointer to the instantiated property.
 *
 * @return pointer property, if one was instantiated,
 *         NULL - otherwise.
 */
ecma_property_t*
ecma_builtin_math_try_to_instantiate_property (ecma_object_t *obj_p, /**< object */
                                               ecma_string_t *prop_name_p) /**< property's name */
{
  JERRY_ASSERT (ecma_builtin_is (obj_p, ECMA_BUILTIN_ID_MATH));
  JERRY_ASSERT (ecma_find_named_property (obj_p, prop_name_p) == NULL);

  ecma_magic_string_id_t id;

  if (!ecma_is_string_magic (prop_name_p, &id))
  {
    return NULL;
  }

  int32_t index = ecma_builtin_bin_search_for_magic_string_id_in_array (ecma_builtin_math_property_names,
                                                                        ecma_builtin_math_property_number,
                                                                        id);

  if (index == -1)
  {
    return NULL;
  }

  JERRY_ASSERT (index >= 0 && (uint32_t) index < sizeof (uint64_t) * JERRY_BITSINBYTE);

  uint32_t bit;
  ecma_internal_property_id_t mask_prop_id;

  if (index >= 32)
  {
    mask_prop_id = ECMA_INTERNAL_PROPERTY_NON_INSTANTIATED_BUILT_IN_MASK_32_63;
    bit = (uint32_t) 1u << (index - 32);
  }
  else
  {
    mask_prop_id = ECMA_INTERNAL_PROPERTY_NON_INSTANTIATED_BUILT_IN_MASK_0_31;
    bit = (uint32_t) 1u << index;
  }

  ecma_property_t *mask_prop_p = ecma_get_internal_property (obj_p, mask_prop_id);
  uint32_t bit_mask = mask_prop_p->u.internal_property.value;

  if (!(bit_mask & bit))
  {
    return NULL;
  }

  bit_mask &= ~bit;

  mask_prop_p->u.internal_property.value = bit_mask;

  ecma_value_t value = ecma_make_simple_value (ECMA_SIMPLE_VALUE_EMPTY);
  ecma_property_writable_value_t writable = ECMA_PROPERTY_WRITABLE;
  ecma_property_enumerable_value_t enumerable = ECMA_PROPERTY_NOT_ENUMERABLE;
  ecma_property_configurable_value_t configurable = ECMA_PROPERTY_CONFIGURABLE;

  switch (id)
  {
#define CASE_ROUTINE_PROP_LIST(name, c_function_name, args_number, length) case name:
    ECMA_BUILTIN_MATH_OBJECT_ROUTINES_PROPERTY_LIST (CASE_ROUTINE_PROP_LIST)
#undef CASE_ROUTINE_PROP_LIST
    {
      ecma_object_t *func_obj_p = ecma_builtin_make_function_object_for_routine (ECMA_BUILTIN_ID_MATH, id);

      value = ecma_make_object_value (func_obj_p);

      break;
    }
#define CASE_VALUE_PROP_LIST(name, value) case name:
    ECMA_BUILTIN_MATH_OBJECT_VALUES_PROPERTY_LIST (CASE_VALUE_PROP_LIST)
#undef CASE_VALUE_PROP_LIST
    {
      writable = ECMA_PROPERTY_NOT_WRITABLE;
      enumerable = ECMA_PROPERTY_NOT_ENUMERABLE;
      configurable = ECMA_PROPERTY_NOT_CONFIGURABLE;

      ecma_number_t *num_p = ecma_alloc_number ();
      value = ecma_make_number_value (num_p);

      switch (id)
      {
#define CASE_VALUE_PROP_LIST(name, value) case name: { *num_p = (ecma_number_t) value; break; }
        ECMA_BUILTIN_MATH_OBJECT_VALUES_PROPERTY_LIST (CASE_VALUE_PROP_LIST)
#undef CASE_VALUE_PROP_LIST
        default:
        {
          JERRY_UNREACHABLE ();
        }
      }

      break;
    }

    default:
    {
      JERRY_UNREACHABLE ();
    }
  }

  ecma_property_t *prop_p = ecma_create_named_data_property (obj_p,
                                                             prop_name_p,
                                                             writable,
                                                             enumerable,
                                                             configurable);

  prop_p->u.named_data_property.value = ecma_copy_value (value, false);
  ecma_gc_update_may_ref_younger_object_flag_by_value (obj_p,
                                                       prop_p->u.named_data_property.value);

  ecma_free_value (value, true);

  return prop_p;
} /* ecma_builtin_math_try_to_instantiate_property */

/**
 * Dispatcher of the Math object's built-in routines
 *
 * @return completion-value
 *         Returned value must be freed with ecma_free_completion_value.
 */
ecma_completion_value_t
ecma_builtin_math_dispatch_routine (ecma_magic_string_id_t builtin_routine_id, /**< Object object's
                                                                                    built-in routine's name */
                                    ecma_value_t arguments_list [], /**< list of arguments passed to routine */
                                    ecma_length_t arguments_number) /**< length of arguments' list */
{
  const ecma_value_t value_undefined = ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED);

  switch (builtin_routine_id)
  {
#define ROUTINE_ARG(n) (arguments_number >= n ? arguments_list[n - 1] : value_undefined)
#define ROUTINE_ARG_LIST_0
#define ROUTINE_ARG_LIST_1 ROUTINE_ARG(1)
#define ROUTINE_ARG_LIST_2 ROUTINE_ARG_LIST_1, ROUTINE_ARG(2)
#define ROUTINE_ARG_LIST_3 ROUTINE_ARG_LIST_2, ROUTINE_ARG(3)
#define ROUTINE_ARG_LIST_NON_FIXED arguments_list, arguments_number
#define CASE_ROUTINE_PROP_LIST(name, c_function_name, args_number, length) \
       case name: \
       { \
         return c_function_name (ROUTINE_ARG_LIST_ ## args_number); \
       }
    ECMA_BUILTIN_MATH_OBJECT_ROUTINES_PROPERTY_LIST (CASE_ROUTINE_PROP_LIST)
#undef CASE_ROUTINE_PROP_LIST
#undef ROUTINE_ARG_LIST_0
#undef ROUTINE_ARG_LIST_1
#undef ROUTINE_ARG_LIST_2
#undef ROUTINE_ARG_LIST_3
#undef ROUTINE_ARG

    default:
    {
      JERRY_UNREACHABLE ();
    }
  }
} /* ecma_builtin_math_dispatch_routine */

/**
 * Get number of routine's parameters
 *
 * @return number of parameters
 */
ecma_length_t
ecma_builtin_math_get_routine_parameters_number (ecma_magic_string_id_t builtin_routine_id) /**< built-in routine's
                                                                                                 name */
{
  switch (builtin_routine_id)
  {
#define CASE_ROUTINE_PROP_LIST(name, c_function_name, args_number, length) case name: return length;
    ECMA_BUILTIN_MATH_OBJECT_ROUTINES_PROPERTY_LIST (CASE_ROUTINE_PROP_LIST)
#undef CASE_ROUTINE_PROP_LIST

    default:
    {
      JERRY_UNREACHABLE ();
    }
  }
} /* ecma_builtin_math_get_routine_parameters_number */

/**
 * @}
 * @}
 * @}
 */
