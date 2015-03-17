/*
 * ====================================================================
 *    Licensed to the Apache Software Foundation (ASF) under one
 *    or more contributor license agreements.  See the NOTICE file
 *    distributed with this work for additional information
 *    regarding copyright ownership.  The ASF licenses this file
 *    to you under the Apache License, Version 2.0 (the
 *    "License"); you may not use this file except in compliance
 *    with the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing,
 *    software distributed under the License is distributed on an
 *    "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *    KIND, either express or implied.  See the License for the
 *    specific language governing permissions and limitations
 *    under the License.
 * ====================================================================
 */

#ifndef SVN_TEST_H
#define SVN_TEST_H

#ifndef SVN_ENABLE_DEPRECATION_WARNINGS_IN_TESTS
#undef SVN_DEPRECATED
#define SVN_DEPRECATED
#endif /* ! SVN_ENABLE_DEPRECATION_WARNINGS_IN_TESTS */

#include <stdio.h>

#include <apr_pools.h>

#include "svn_delta.h"
#include "svn_path.h"
#include "svn_types.h"
#include "svn_error.h"
#include "svn_string.h"
#include "svn_auth.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** Handy macro to test a condition, returning SVN_ERR_TEST_FAILED if FALSE
 *
 * This macro should be used in place of SVN_ERR_ASSERT() since we don't
 * want to core-dump the test.
 */
#define SVN_TEST_ASSERT(expr)                                     \
  do {                                                            \
    if (!(expr))                                                  \
      return svn_error_createf(SVN_ERR_TEST_FAILED, NULL,         \
                               "assertion '%s' failed at %s:%d",  \
                               #expr, __FILE__, __LINE__);        \
  } while (0)

/**
 * Macro for testing assumptions when the context does not allow
 * returning an svn_error_t*.
 *
 * Will write to stderr and cause a segfault if EXPR is false.
 */
#define SVN_TEST_ASSERT_NO_RETURN(expr)                           \
  do {                                                            \
    if (!(expr))                                                  \
      {                                                           \
        unsigned int z_e_r_o_p_a_g_e__;                           \
        fprintf(stderr, "TEST ASSERTION FAILED: %s\n", #expr);    \
        z_e_r_o_p_a_g_e__ = *(volatile unsigned int*)0;           \
        *(volatile unsigned int*)0 = z_e_r_o_p_a_g_e__;           \
      }                                                           \
  } while (0)

/** Handy macro for testing an expected svn_error_t return value.
 * EXPECTED must be a real error (neither SVN_NO_ERROR nor APR_SUCCESS).
 * The error returned by EXPR will be cleared.
 */
#define SVN_TEST_ASSERT_ERROR(expr, expected)                             \
  do {                                                                    \
    svn_error_t *err__ = (expr);                                          \
    SVN_ERR_ASSERT((expected));                                           \
    if (err__ == SVN_NO_ERROR || err__->apr_err != (expected))            \
      return err__ ? svn_error_createf(SVN_ERR_TEST_FAILED, err__,        \
                                       "Expected error %s but got %s",    \
                                       svn_error_symbolic_name(expected), \
                                       svn_error_symbolic_name(           \
                                         err__->apr_err))                 \
                   : svn_error_createf(SVN_ERR_TEST_FAILED, err__,        \
                                       "Expected error %s but got %s",    \
                                       svn_error_symbolic_name(expected), \
                                        "SVN_NO_ERROR");                  \
    svn_error_clear(err__);                                               \
  } while (0)

/** Handy macro for testing that an svn_error_t is returned.
 * The result must be neither SVN_NO_ERROR nor SVN_ERR_ASSERTION_FAIL.
 * The error returned by EXPR will be cleared.
 */
#define SVN_TEST_ASSERT_ANY_ERROR(expr)                                   \
  do {                                                                    \
    svn_error_t *err__ = (expr);                                          \
    if (err__ == SVN_NO_ERROR || err__->apr_err == SVN_ERR_ASSERTION_FAIL)\
      return err__ ? svn_error_createf(SVN_ERR_TEST_FAILED, err__,        \
                                       "Expected error but got %s",       \
                                       "SVN_ERR_ASSERTION_FAIL")          \
                   : svn_error_createf(SVN_ERR_TEST_FAILED, err__,        \
                                       "Expected error but got %s",       \
                                       "SVN_NO_ERROR");                   \
    svn_error_clear(err__);                                               \
  } while (0)

/** Handy macro for testing string equality.
 *
 * EXPR and/or EXPECTED_EXPR may be NULL which compares equal to NULL and
 * not equal to any non-NULL string.
 */
#define SVN_TEST_STRING_ASSERT(expr, expected_expr)                 \
  do {                                                              \
    const char *tst_str1 = (expr);                                  \
    const char *tst_str2 = (expected_expr);                         \
                                                                    \
    if (tst_str2 == NULL && tst_str1 == NULL)                       \
      break;                                                        \
    if ((tst_str1 == NULL) || (tst_str2 == NULL)                    \
        || (strcmp(tst_str2, tst_str1) != 0))                       \
      return svn_error_createf(SVN_ERR_TEST_FAILED, NULL,           \
          "Strings not equal\n  Expected: '%s'\n  Found:    '%s'"   \
          "\n  at %s:%d",                                           \
          tst_str2, tst_str1, __FILE__, __LINE__);                  \
  } while(0)


/* Baton for any arguments that need to be passed from main() to svn
 * test functions.
 */
typedef struct svn_test_opts_t
{
  /* The name of the application (to generate unique names) */
  const char *prog_name;
  /* Description of the fs backend that should be used for testing. */
  const char *fs_type;
  /* Config file. */
  const char *config_file;
  /* Source dir. */
  const char *srcdir;
  /* Repository dir: temporary directory to create repositories in as subdir */
  const char *repos_dir;
  /* Repository url: The url to access REPOS_DIR as */
  const char *repos_url;
  /* Repository template: pre-created repository to copy for tests */
  const char *repos_template;
  /* Minor version to use for servers and FS backends, or zero to use
     the current latest version. */
  int server_minor_version;
  svn_boolean_t verbose;
  /* Add future "arguments" here. */
} svn_test_opts_t;

/* Prototype for test driver functions. */
typedef svn_error_t* (*svn_test_driver2_t)(apr_pool_t *pool);

/* Prototype for test driver functions which need options. */
typedef svn_error_t* (*svn_test_driver_opts_t)(const svn_test_opts_t *opts,
                                               apr_pool_t *pool);

/* Prototype for test predicate functions. */
typedef svn_boolean_t (*svn_test_predicate_func_t)(const svn_test_opts_t *opts,
                                                   const char *predicate_value,
                                                   apr_pool_t *pool);

/* Test modes. */
enum svn_test_mode_t
  {
    svn_test_pass,
    svn_test_xfail,
    svn_test_skip,
    svn_test_all
  };

/* Structure for runtime test predicates. */
struct svn_test_predicate_t
{
  /* The predicate function. */
  svn_test_predicate_func_t func;

  /* The value that the predicate function tests. */
  const char *value;

  /* The test mode that's used if the predicate matches. */
  enum svn_test_mode_t alternate_mode;

  /* Description for the test log */
  const char *description;
};


/* Each test gets a test descriptor, holding the function and other
 * associated data.
 */
struct svn_test_descriptor_t
{
  /* Is the test marked XFAIL? */
  enum svn_test_mode_t mode;

  /* A pointer to the test driver function. */
  svn_test_driver2_t func2;

  /* A pointer to the test driver function. */
  svn_test_driver_opts_t func_opts;

  /* A descriptive message for this test. */
  const char *msg;

  /* An optional description of a work-in-progress test. */
  const char *wip;

  /* An optional runtiume predicate. */
  struct svn_test_predicate_t predicate;
};

/* All Subversion test programs include an array of svn_test_descriptor_t's
 * (all of our sub-tests) that begins and ends with a SVN_TEST_NULL entry.
 * This descriptor must be passed to the svn_test_main function.
 *
 * MAX_THREADS is the number of concurrent tests to run.  Set to 1 if
 * all tests must be executed serially.  Numbers less than 1 mean
 * "unbounded".
 */
int svn_test_main(int argc, const char *argv[], int max_threads,
                  struct svn_test_descriptor_t *test_funcs);

/* Boilerplate for the main function for each test program. */
#define SVN_TEST_MAIN                                 \
  int main(int argc, const char *argv[])              \
    {                                                 \
      return svn_test_main(argc, argv,                \
                           max_threads, test_funcs);  \
    }

/* A null initializer for the test descriptor. */
#define SVN_TEST_NULL  {0}

/* Initializer for PASS tests */
#define SVN_TEST_PASS2(func, msg)  {svn_test_pass, func, NULL, msg}

/* Initializer for XFAIL tests */
#define SVN_TEST_XFAIL2(func, msg) {svn_test_xfail, func, NULL, msg}

/* Initializer for conditional XFAIL tests */
#define SVN_TEST_XFAIL_COND2(func, p, msg) \
  {(p) ? svn_test_xfail : svn_test_pass, func, NULL, msg}

/* Initializer for SKIP tests */
#define SVN_TEST_SKIP2(func, p, msg) \
  {(p) ? svn_test_skip : svn_test_pass, func, NULL, msg}

/* Similar macros, but for tests needing options.  */
#define SVN_TEST_OPTS_PASS(func, msg)  {svn_test_pass, NULL, func, msg}
#define SVN_TEST_OPTS_XFAIL(func, msg) {svn_test_xfail, NULL, func, msg}
#define SVN_TEST_OPTS_XFAIL_COND(func, p, msg) \
  {(p) ? svn_test_xfail : svn_test_pass, NULL, func, msg}
#define SVN_TEST_OPTS_XFAIL_OTOH(func, msg, predicate) \
  {svn_test_xfail, NULL, func, msg, NULL, predicate}
#define SVN_TEST_OPTS_SKIP(func, p, msg) \
  {(p) ? svn_test_skip : svn_test_pass, NULL, func, msg}

/* Initializer for XFAIL tests for works-in-progress. */
#define SVN_TEST_WIMP(func, msg, wip) \
  {svn_test_xfail, func, NULL, msg, wip}
#define SVN_TEST_WIMP_COND(func, p, msg, wip) \
  {(p) ? svn_test_xfail : svn_test_pass, func, NULL, msg, wip}
#define SVN_TEST_OPTS_WIMP(func, msg, wip) \
  {svn_test_xfail, NULL, func, msg, wip}
#define SVN_TEST_OPTS_WIMP_COND(func, p, msg, wip) \
  {(p) ? svn_test_xfail : svn_test_pass, NULL, func, msg, wip}


/* Return a pseudo-random number based on SEED, and modify SEED.
 *
 * This is a "good" pseudo-random number generator, intended to replace
 * all those "bad" rand() implementations out there.
 */
apr_uint32_t svn_test_rand(apr_uint32_t *seed);


/* Add PATH to the test cleanup list.  */
void svn_test_add_dir_cleanup(const char *path);


/* A simple representation for a tree node. */
typedef struct svn_test__tree_entry_t
{
  const char *path;     /* relpath of this node */
  const char *contents; /* text contents, or NULL for a directory */
}
svn_test__tree_entry_t;

/* Wrapper for an array of svn_test__tree_entry_t's. */
typedef struct svn_test__tree_t
{
  svn_test__tree_entry_t *entries;
  int num_entries;
}
svn_test__tree_t;


/* The standard Greek tree, terminated by a node with path=NULL. */
extern const svn_test__tree_entry_t svn_test__greek_tree_nodes[21];


/* Returns a path to BASENAME within the transient data area for the
   current test. */
const char *
svn_test_data_path(const char* basename, apr_pool_t *result_pool);


/* Some tests require the --srcdir option and should use this function
 * to get it. If not provided, print a warning and attempt to run the
 * tests under the assumption that --srcdir is the current directory. */
svn_error_t *
svn_test_get_srcdir(const char **srcdir,
                    const svn_test_opts_t *opts,
                    apr_pool_t *pool);

/* Initializes a standard auth baton for accessing the repositories */
svn_error_t *
svn_test__init_auth_baton(svn_auth_baton_t **baton,
                          apr_pool_t *result_pool);


/*
 * Test predicates
 */

#define SVN_TEST_PASS_IF_FS_TYPE_IS(fs_type) \
  { svn_test__fs_type_is, fs_type, svn_test_pass, \
    "PASS if fs-type = " fs_type }

#define SVN_TEST_PASS_IF_FS_TYPE_IS_NOT(fs_type) \
  { svn_test__fs_type_not, fs_type, svn_test_pass, \
    "PASS if fs-type != " fs_type }

/* Return TRUE if the fs-type in OPTS matches PREDICATE_VALUE. */
svn_boolean_t
svn_test__fs_type_is(const svn_test_opts_t *opts,
                     const char *predicate_value,
                     apr_pool_t *pool);


/* Return TRUE if the fs-type in OPTS does not matches PREDICATE_VALUE. */
svn_boolean_t
svn_test__fs_type_not(const svn_test_opts_t *opts,
                      const char *predicate_value,
                      apr_pool_t *pool);

void
svn_test__enable_sleep_for_timestamps(apr_pool_t *pool);

void
svn_test__disable_sleep_for_timestamps(apr_pool_t *pool);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SVN_TEST_H */
