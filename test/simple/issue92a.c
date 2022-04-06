/*
 * Copyright 2018 Jeffrey Kegler
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/* Tests of Libmarpa methods on trivial grammar */

#include <stdio.h>
#include <string.h>
#include "marpa.h"

#include "marpa_m_test.h"

static int
warn (const char *s, Marpa_Grammar g)
{
  printf ("%s returned %d\n", s, marpa_g_error (g, NULL));
}

static int
fail (const char *s, Marpa_Grammar g)
{
  warn (s, g);
  exit (1);
}

Marpa_Symbol_ID S_top;
Marpa_Symbol_ID S_A1;
Marpa_Symbol_ID S_A2;
Marpa_Symbol_ID S_B1;
Marpa_Symbol_ID S_B2;
Marpa_Symbol_ID S_C1;
Marpa_Symbol_ID S_C2;

/* Longest rule is <= 4 symbols */
Marpa_Symbol_ID rhs[4];

Marpa_Rule_ID R_top_1;
Marpa_Rule_ID R_top_2;
Marpa_Rule_ID R_C2_3; // highest rule id

/* For (error) messages */
char msgbuf[80];

char *
symbol_name (Marpa_Symbol_ID id)
{
  if (id == S_top) return "top";
  if (id == S_A1) return "A1";
  if (id == S_A2) return "A2";
  if (id == S_B1) return "B1";
  if (id == S_B2) return "B2";
  if (id == S_C1) return "C1";
  if (id == S_C2) return "C2";
  sprintf (msgbuf, "no such symbol: %d", id);
  return msgbuf;
}

int
is_nullable (Marpa_Symbol_ID id)
{
  if (id == S_top) return 1;
  if (id == S_A1) return 1;
  if (id == S_A2) return 1;
  if (id == S_B1) return 1;
  if (id == S_B2) return 1;
  if (id == S_C1) return 1;
  if (id == S_C2) return 1;
  return 0;
}

int
is_nulling (Marpa_Symbol_ID id)
{
  if (id == S_C1) return 1;
  if (id == S_C2) return 1;
  return 0;
}

static Marpa_Grammar
marpa_g_trivial_new(Marpa_Config *config)
{
  Marpa_Grammar g;
  g = marpa_g_new (config);
  if (!g)
    {
      Marpa_Error_Code errcode = marpa_c_error (config, NULL);
      printf ("marpa_g_new returned %d", errcode);
      exit (1);
    }

  ((S_top = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_A1 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_A2 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_B1 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_B2 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_C1 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_C2 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);

  rhs[0] = S_A1;
  ((R_top_1 = marpa_g_rule_new (g, S_top, rhs, 1)) >= 0)
    || fail ("marpa_g_rule_new", g);
  rhs[0] = S_A2;
  ((R_top_2 = marpa_g_rule_new (g, S_top, rhs, 1)) >= 0)
    || fail ("marpa_g_rule_new", g);
  rhs[0] = S_B1;
  (marpa_g_rule_new (g, S_A1, rhs, 1) >= 0)
    || fail ("marpa_g_rule_new", g);
  rhs[0] = S_B2;
  (marpa_g_rule_new (g, S_A2, rhs, 1) >= 0)
    || fail ("marpa_g_rule_new", g);
  rhs[0] = S_C1;
  (marpa_g_rule_new (g, S_B1, rhs, 1) >= 0)
    || fail ("marpa_g_rule_new", g);
  rhs[0] = S_C2;
  (marpa_g_rule_new (g, S_B2, rhs, 1) >= 0)
    || fail ("marpa_g_rule_new", g);
  (marpa_g_rule_new (g, S_C1, rhs, 0) >= 0)
    || fail ("marpa_g_rule_new", g);

  ((R_C2_3 = marpa_g_rule_new (g, S_C2, rhs, 0)) >= 0)
    || fail ("marpa_g_rule_new", g);

  return g;
}

static Marpa_Error_Code
marpa_g_trivial_precompute(Marpa_Grammar g, Marpa_Symbol_ID S_start)
{
  Marpa_Error_Code rc;

  (marpa_g_start_symbol_set (g, S_start) >= 0)
    || fail ("marpa_g_start_symbol_set", g);

  rc = marpa_g_precompute (g);
  if (rc < 0)
    fail("marpa_g_precompute", g);

  return rc;
}

static void defaults_reset(API_test_data* defaults, Marpa_Grammar g)
{
  defaults->g = g;
  defaults->expected_errcode = MARPA_ERR_NONE;
  defaults->msg = "";
  defaults->rv_seen.int_rv = -86;
}

int
main (int argc, char *argv[])
{
  int rc;
  int ix;

  /* For the test of marpa_r_earley_set_values() */
  const int orig_int_value = 1729;
  int int_value = orig_int_value;

  Marpa_Config marpa_configuration;

  Marpa_Grammar g;
  Marpa_Recognizer r;

  Marpa_Rank negative_rank, positive_rank;
  int flag;

  int whatever;

  char *value2_base = NULL;
  void *value2 = value2_base;

  API_test_data defaults;
  API_test_data this_test;

  plan(120);

  marpa_c_init (&marpa_configuration);
  g = marpa_g_trivial_new(&marpa_configuration);
  defaults_reset(&defaults, g);
  this_test = defaults;

  /* precomputation */
  marpa_g_trivial_precompute(g, S_top);
  ok(1, "precomputation succeeded");

  API_STD_TEST0(defaults, 0, MARPA_ERR_NONE, marpa_g_force_valued, g);

  /* Recognizer Methods */
  {
    r = marpa_r_new (g);
    if (!r)
      fail("marpa_r_new", g);

    /* the recce hasn't been started yet */

    API_STD_TEST0(defaults, -1, MARPA_ERR_RECCE_NOT_STARTED, marpa_r_current_earleme, r);
    API_STD_TEST0(defaults, -2, MARPA_ERR_RECCE_NOT_STARTED, marpa_r_progress_report_reset, r);
    API_STD_TEST1(defaults, -2, MARPA_ERR_RECCE_NOT_STARTED, marpa_r_progress_report_start, r, whatever);
    API_STD_TEST0(defaults, -2, MARPA_ERR_RECCE_NOT_STARTED, marpa_r_progress_report_finish, r);

    {
      int set_id;
      Marpa_Earley_Set_ID origin;
      API_STD_TEST2(defaults, -2, MARPA_ERR_RECCE_NOT_STARTED,
	marpa_r_progress_item, r, &set_id, &origin);
    }

    /* start the recce */
    rc = marpa_r_start_input (r);
    if (!rc)
      fail("marpa_r_start_input", g);

    diag ("The below recce tests are at earleme 0");

    { /* event loop -- just count events so far -- there must be no event except exhausted */
      Marpa_Event event;
      int exhausted_event_triggered = 0;
      int spurious_events = 0;
      int prediction_events = 0;
      int completion_events = 0;
      int event_ix;
      const int event_count = marpa_g_event_count (g);

      is_int(1, event_count, "event count at earleme 0 is %ld", (long) event_count);

      for (event_ix = 0; event_ix < event_count; event_ix++)
      {
        int event_type = marpa_g_event (g, &event, event_ix);
        if (event_type == MARPA_EVENT_SYMBOL_COMPLETED)
          completion_events++;
        else if (event_type == MARPA_EVENT_SYMBOL_PREDICTED)
          prediction_events++;
        else if (event_type == MARPA_EVENT_EXHAUSTED)
          exhausted_event_triggered++;
        else
        {
          printf ("spurious event type is %ld\n", (long) event_type);
          spurious_events++;
        }
      }

      is_int(0, spurious_events, "spurious events triggered: %ld", (long) spurious_events);
      is_int(0, completion_events, "completion events triggered: %ld", (long) completion_events);
      is_int(0, prediction_events, "completion events triggered: %ld", (long) prediction_events);
      ok (exhausted_event_triggered, "exhausted event triggered");

    } /* event loop */

    /* recognizer reading methods */
    Marpa_Symbol_ID S_token = S_A2;
    this_test.msg = "not accepting input is checked before invalid symbol";
    API_CODE_TEST3(this_test, MARPA_ERR_RECCE_NOT_ACCEPTING_INPUT,
        marpa_r_alternative, r, S_invalid, 0, 0);

    this_test.msg = "not accepting input is checked before no such symbol";
    API_CODE_TEST3(this_test, MARPA_ERR_RECCE_NOT_ACCEPTING_INPUT,
        marpa_r_alternative, r, S_no_such, 0, 0);

    this_test.msg = "not accepting input";
    API_CODE_TEST3(this_test, MARPA_ERR_RECCE_NOT_ACCEPTING_INPUT,
        marpa_r_alternative, r, S_token, 0, 0);

    this_test = defaults;

    API_STD_TEST0(defaults, -2, MARPA_ERR_RECCE_NOT_ACCEPTING_INPUT,
	marpa_r_earleme_complete, r);

    this_test.msg = "at earleme 0";
    API_STD_TEST0(this_test, 1, MARPA_ERR_NONE, marpa_r_is_exhausted, r);

    /* Location accessors */
    {
      /* the below 2 always succeed */
      unsigned int current_earleme = 0;
      API_STD_TEST0(defaults, current_earleme, MARPA_ERR_NONE, marpa_r_current_earleme, r);

      unsigned int furthest_earleme = current_earleme;
      API_STD_TEST0(defaults, furthest_earleme, MARPA_ERR_NONE, marpa_r_furthest_earleme, r);

      API_STD_TEST0(defaults, furthest_earleme, MARPA_ERR_NONE, marpa_r_latest_earley_set, r);

      API_STD_TEST1(defaults, current_earleme, MARPA_ERR_NONE,
	marpa_r_earleme, r, current_earleme);

      API_STD_TEST1(defaults, -1, MARPA_ERR_RECCE_NOT_ACCEPTING_INPUT,
	marpa_r_earley_set_value, r, current_earleme);

      /* marpa_r_earley_set_value_*() methods */
      int taxicab = 1729;

      int earley_set;

      struct marpa_r_earley_set_value_test {
        int earley_set;

        int rv_marpa_r_earleme;
        int rv_marpa_r_latest_earley_set_value_set;
        int rv_marpa_r_earley_set_value;
        int rv_marpa_r_latest_earley_set_values_set;

        int   rv_marpa_r_earley_set_values;
        int   int_p_value_rv_marpa_r_earley_set_values;
        void* void_p_value_rv_marpa_r_earley_set_values;

        Marpa_Error_Code errcode;
      };
      typedef struct marpa_r_earley_set_value_test Marpa_R_Earley_Set_Value_Test;

      const Marpa_R_Earley_Set_Value_Test tests[] = {
        { -1, -2, taxicab,      -2, 1, -2, taxicab, value2, MARPA_ERR_INVALID_LOCATION },
        {  0,  0, taxicab, taxicab, 1,  1,      42, value2, MARPA_ERR_INVALID_LOCATION },
        {  1, -2,      42,      -2, 1, -2,      42, value2, MARPA_ERR_NO_EARLEY_SET_AT_LOCATION },
        {  2, -2,      42,      -2, 1, -2,      42, value2, MARPA_ERR_NO_EARLEY_SET_AT_LOCATION },
      };

      for (ix = 0; ix < sizeof(tests) / sizeof(Marpa_R_Earley_Set_Value_Test); ix++)
        {
          const Marpa_R_Earley_Set_Value_Test t = tests[ix];
          diag("marpa_r_earley_set_value_*() methods, earley_set: %d", t.earley_set);

          if (t.earley_set == -1 || t.earley_set == 1 || t.earley_set == 2) {
	    API_STD_TEST1(defaults, t.rv_marpa_r_earleme, t.errcode,
	      marpa_r_earleme, r, t.earley_set);
          } else {
	    API_STD_TEST1(defaults, t.rv_marpa_r_earleme, MARPA_ERR_NONE,
	      marpa_r_earleme, r, t.earley_set);
	  }

	  API_STD_TEST1(defaults, t.rv_marpa_r_latest_earley_set_value_set, MARPA_ERR_NONE,
	    marpa_r_latest_earley_set_value_set, r, t.rv_marpa_r_latest_earley_set_value_set);

          if (t.earley_set == -1 || t.earley_set == 1 || t.earley_set == 2) {
	    API_STD_TEST1(defaults, t.rv_marpa_r_earley_set_value, t.errcode,
	      marpa_r_earley_set_value, r, t.earley_set);
          } else {
	    API_STD_TEST1(defaults, t.rv_marpa_r_earley_set_value, MARPA_ERR_NONE,
	      marpa_r_earley_set_value, r, t.earley_set);
	  }

	  {
	    API_STD_TEST2(defaults,
	      t.rv_marpa_r_latest_earley_set_values_set,
	      MARPA_ERR_NONE,
	      marpa_r_latest_earley_set_values_set,
	      r,
	      42, value2);
	  }

	  {
	    /* There is no c89 portable way to test arbitrary pointers.
	     * With ifdef's we could cover 99.999% of cases, but for now
	     * we do not bother.
	     */
	    void *orig_value2 = NULL;
	    void *value2 = orig_value2;

	    API_STD_TEST3 (defaults,
			   t.rv_marpa_r_earley_set_values,
			   t.errcode,
			   marpa_r_earley_set_values,
			   r, t.earley_set, (&int_value), &value2);
	    is_int (t.int_p_value_rv_marpa_r_earley_set_values,
		    int_value, "marpa_r_earley_set_values() int* value");

	  }

        }
    } /* Location Accessors */

    /* Other parse status methods */
    {
      int boolean = 0;
      int threshold = 1;
      API_STD_TEST1(defaults, threshold, MARPA_ERR_NONE,
	marpa_r_earley_item_warning_threshold_set, r, threshold);

      API_STD_TEST0(defaults, threshold, MARPA_ERR_NONE, marpa_r_earley_item_warning_threshold, r);
    }

    {
      Marpa_Symbol_ID S_expected = S_C1;
      int value = 1;
      API_STD_TEST2(defaults, -2, MARPA_ERR_SYMBOL_IS_NULLING, marpa_r_expected_symbol_event_set, r, S_B1, value);

      {
	Marpa_Symbol_ID buffer[42];
	API_STD_TEST1(defaults, 0, MARPA_ERR_NONE, marpa_r_terminals_expected, r, buffer);
      }

      API_STD_TEST1(defaults, 0, MARPA_ERR_NONE,
	marpa_r_terminal_is_expected, r, S_C1);
      API_STD_TEST1(defaults, -2, MARPA_ERR_INVALID_SYMBOL_ID,
	marpa_r_terminal_is_expected, r, S_invalid);
      API_STD_TEST1(defaults, -2, MARPA_ERR_NO_SUCH_SYMBOL_ID,
	marpa_r_terminal_is_expected, r, S_no_such);

    } /* Other parse status methods */

    /* Progress reports */
    {
      API_STD_TEST0(defaults, -2, MARPA_ERR_PROGRESS_REPORT_NOT_STARTED,
          marpa_r_progress_report_reset, r);

      API_STD_TEST0(defaults, -2, MARPA_ERR_PROGRESS_REPORT_NOT_STARTED,
          marpa_r_progress_report_finish, r);

      {
	int set_id;
	Marpa_Earley_Set_ID origin;
	API_STD_TEST2(defaults, -2, MARPA_ERR_PROGRESS_REPORT_NOT_STARTED,
	  marpa_r_progress_item, r, &set_id, &origin);
      }


      /* start report at bad locations */
      Marpa_Earley_Set_ID ys_id_negative = -1;
      API_STD_TEST1(defaults, -2, MARPA_ERR_INVALID_LOCATION,
          marpa_r_progress_report_start, r, ys_id_negative);

      Marpa_Earley_Set_ID ys_id_not_existing = 1;
      API_STD_TEST1(defaults, -2, MARPA_ERR_NO_EARLEY_SET_AT_LOCATION,
          marpa_r_progress_report_start, r, ys_id_not_existing);

      /* start report at earleme 0 */
      Marpa_Earley_Set_ID earleme_0 = 0;
      this_test.msg = "no items at earleme 0";
      API_STD_TEST1(this_test, 0, MARPA_ERR_NONE,
          marpa_r_progress_report_start, r, earleme_0);

      {
	int set_id;
	Marpa_Earley_Set_ID origin;
	API_STD_TEST2(defaults, -1, MARPA_ERR_PROGRESS_REPORT_EXHAUSTED,
	  marpa_r_progress_item, r, &set_id, &origin);
      }


      int non_negative_value = 1;
      API_STD_TEST0(this_test, non_negative_value, MARPA_ERR_NONE,
          marpa_r_progress_report_reset, r);

      this_test.msg = "at earleme 0";
      API_STD_TEST0(this_test, non_negative_value, MARPA_ERR_NONE,
          marpa_r_progress_report_finish, r);
    }

    /* Bocage, Order, Tree, Value */
    {
      /* Bocage */
      Marpa_Earley_Set_ID ys_invalid = -2;
      API_PTR_TEST1(defaults, MARPA_ERR_INVALID_LOCATION,
          marpa_b_new, r, ys_invalid);

      Marpa_Earley_Set_ID ys_non_existing = 1;
      API_PTR_TEST1(defaults, MARPA_ERR_NO_PARSE,
          marpa_b_new, r, ys_non_existing);

      Marpa_Earley_Set_ID ys_at_current_earleme = -1;
      Marpa_Bocage b = marpa_b_new(r, ys_at_current_earleme);
      if (!b)
        fail("marpa_b_new", g);
      else
        ok(1, "marpa_b_new(): parse at current earleme of trivial parse");

      marpa_b_unref(b);

      b = marpa_b_new(r, 0);

      if (!b)
        fail("marpa_b_new", g);
      else
        ok(1, "marpa_b_new(): null parse at earleme 0");

      API_STD_TEST0(defaults, 1, MARPA_ERR_NONE,
	  marpa_b_ambiguity_metric, b);
      API_STD_TEST0(defaults, 1, MARPA_ERR_NONE,
	  marpa_b_is_null, b);

      /* Order */
      Marpa_Order o = marpa_o_new (b);

      if (!o)
        fail("marpa_o_new", g);
      else
        ok(1, "marpa_o_new() at earleme 0");

      int flag = 1;
      API_STD_TEST1(defaults, flag, MARPA_ERR_NONE,
	  marpa_o_high_rank_only_set, o, flag);
      API_STD_TEST0(defaults, flag, MARPA_ERR_NONE,
	  marpa_o_high_rank_only, o);

      API_STD_TEST0(defaults, 1, MARPA_ERR_NONE,
	  marpa_o_ambiguity_metric, o);
      API_STD_TEST0(defaults, 1, MARPA_ERR_NONE,
	  marpa_o_is_null, o);

      API_STD_TEST1(defaults, -2, MARPA_ERR_ORDER_FROZEN,
	  marpa_o_high_rank_only_set, o, flag);
      API_STD_TEST0(defaults, flag, MARPA_ERR_NONE,
	  marpa_o_high_rank_only, o);

      /* Tree */
      Marpa_Tree t;

      t = marpa_t_new (o);
      if (!t)
        fail("marpa_t_new", g);
      else
        ok(1, "marpa_t_new() at earleme 0");

      this_test.msg = "before the first parse tree";
      API_STD_TEST0(this_test, 0, MARPA_ERR_NONE, marpa_t_parse_count, t);
      API_STD_TEST0(defaults, 0, MARPA_ERR_NONE, marpa_t_next, t);

      /* Value */
      Marpa_Value v = marpa_v_new(t);
      if (!t)
        fail("marpa_v_new", g);
      else
        ok(1, "marpa_v_new() at earleme 0");

      {
        Marpa_Step_Type step_type = marpa_v_step (v);
	is_int(MARPA_STEP_NULLING_SYMBOL, step_type, "MARPA_STEP_NULLING_SYMBOL step.");

	is_int(0, marpa_v_result(v), "marpa_v_result(v)");
	is_int(MARPA_STEP_NULLING_SYMBOL, marpa_v_step_type(v), "marpa_v_step_type(v)");
	is_int(0, marpa_v_symbol(v), "marpa_v_symbol(v)");
	is_int(-1, marpa_v_es_id(v), "marpa_v_es_id(v)");
	is_int(-1, marpa_v_token_start_es_id(v), "marpa_v_token_start_es_id(v)");

        step_type = marpa_v_step (v);
	is_int(MARPA_STEP_INACTIVE, step_type, "MARPA_STEP_INACTIVE step.");
      }

      API_STD_TEST0(defaults, 1, MARPA_ERR_NONE, marpa_t_parse_count, t);
      API_STD_TEST0(defaults, -2, MARPA_ERR_TREE_PAUSED, marpa_t_next, t);

      marpa_v_unref(v);

      API_STD_TEST0(defaults, 1, MARPA_ERR_NONE, marpa_t_parse_count, t);
      API_STD_TEST0(defaults, -1, MARPA_ERR_TREE_EXHAUSTED, marpa_t_next, t);

    } /* Bocage, Order, Tree, Value */

  } /* recce method tests */

  return 0;
}
