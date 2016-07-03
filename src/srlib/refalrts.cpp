#include <exception>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <assert.h>

#include "refalrts.h"

#if 1
#  define VALID_LINKED( iter )
#else
#  define VALID_LINKED( iter ) valid_linked_aux( #iter, iter );
#endif

#ifndef SHOW_DEBUG
#define SHOW_DEBUG 0
#endif // ifdef SHOW_DEBUG

enum { SWITCH_DEFAULT_VIOLATION = 0 };

void valid_linked_aux( const char *text, refalrts::Iter i ) {
  printf("checking %s\n", text);
  if( 0 == i ) {
    return;
  }

  if( i->next ) {
    assert( i->next->prev == i );
  }

  if( i->prev ) {
    assert( i->prev->next == i );
  }
}

//==============================================================================
// Примитивные операции
//==============================================================================

// Операции распознавания

void refalrts::use( refalrts::Iter& ) {
  /* Ничего не делаем. Эта функция добавляется, чтобы подавить предупреждение
  компилятора о том, что переменная не используется */;
}

void refalrts::zeros(refalrts::Iter context[], int size){
  for (int i = 0; i < size; i++) {
    context[i] = 0;
  }
}

void refalrts::use_counter( unsigned& ) {
  /* Ничего не делаем. Эта функция добавляется, чтобы подавить предупреждение
  компилятора о том, что переменная не используется */;
}

namespace {

refalrts::Iter next( refalrts::Iter current ) {
  return current->next;
}

refalrts::Iter prev( refalrts::Iter current ) {
  return current->prev;
}

bool is_open_bracket( refalrts::Iter node ) {
  return (refalrts::cDataOpenBracket == node->tag)
    || (refalrts::cDataOpenADT == node->tag);
}

bool is_close_bracket( refalrts::Iter node ) {
  return (refalrts::cDataCloseBracket == node->tag)
    || (refalrts::cDataCloseADT == node->tag);
}

} // unnamed namespace

void refalrts::move_left(
  refalrts::Iter& first, refalrts::Iter& last
) {
  //assert( (first == 0) == (last == 0) );
  if( first == 0 ) assert (last == 0);
  if( first != 0 ) assert (last != 0);

  if( first == last ) {
    first = 0;
    last = 0;
  } else {
    first = next( first );
  }
}

void refalrts::move_right(
  refalrts::Iter& first, refalrts::Iter& last
) {
  //assert( (first == 0) == (last == 0) );
  if( first == 0 ) assert (last == 0);
  if( first != 0 ) assert (last != 0);

  if( first == last ) {
    first = 0;
    last = 0;
  } else {
    last = prev( last );
  }
}

bool refalrts::empty_seq( refalrts::Iter first, refalrts::Iter last ) {
  //assert( (first == 0) == (last == 0) );
  if( first == 0 ) assert (last == 0);
  if( first != 0 ) assert (last != 0);

  return (first == 0) && (last == 0);
}

bool refalrts::function_term(
  refalrts::RefalFunctionPtr func, refalrts::Iter pos
) {
  return (pos->tag == cDataFunction) && (pos->function_info.ptr == func);
}

refalrts::Iter refalrts::function_left(
  refalrts::RefalFunctionPtr fn, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if( first->tag != cDataFunction ) {
    return 0;
  } else if ( first->function_info.ptr != fn ) {
    return 0;
  } else {
    Iter func_pos = first;
    move_left( first, last );
    return func_pos;
  }
}

refalrts::Iter refalrts::function_right(
  refalrts::RefalFunctionPtr fn, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if ( cDataFunction != last->tag ) {
    return 0;
  } else if ( last->function_info.ptr != fn ) {
    return 0;
  } else {
    Iter func_pos = last;
    move_right( first, last );
    return func_pos;
  }
}

bool refalrts::char_term(char ch, refalrts::Iter& pos) {
  return (pos->tag == cDataChar) && (pos->char_info == ch);
}

refalrts::Iter refalrts::char_left(
  char ch, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if ( cDataChar != first->tag ) {
    return 0;
  } else if ( first->char_info != ch ) {
    return 0;
  } else {
    Iter char_pos = first;
    move_left( first, last );
    return char_pos;
  }
}

refalrts::Iter refalrts::char_right(
  char ch, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if ( cDataChar != last->tag ) {
    return 0;
  } else if ( last->char_info != ch ) {
    return 0;
  } else {
    Iter char_pos = last;
    move_right( first, last );
    return char_pos;
  }
}


bool refalrts::number_term(
  refalrts::RefalNumber num, refalrts::Iter& pos
) {
    return (pos->tag == cDataNumber) && (pos->number_info == num);
}

refalrts::Iter refalrts::number_left(
  refalrts::RefalNumber num, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if ( cDataNumber != first->tag ) {
    return 0;
  } else if ( first->number_info != num ) {
    return 0;
  } else {
    Iter num_pos = first;
    move_left( first, last );
    return num_pos;
  }
}

refalrts::Iter refalrts::number_right(
  refalrts::RefalNumber num, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if ( cDataNumber != last->tag ) {
    return 0;
  } else if ( last->number_info != num ) {
    return 0;
  } else {
    Iter num_pos = last;
    move_right( first, last );
    return num_pos;
  }
}

bool refalrts::ident_term(
  refalrts::RefalIdentifier ident, refalrts::Iter& pos
) {
    return (pos->tag == cDataIdentifier) && (pos->ident_info == ident);
}

refalrts::Iter refalrts::ident_left(
  refalrts::RefalIdentifier ident, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if ( cDataIdentifier != first->tag ) {
    return 0;
  } else if ( first->ident_info != ident ) {
    return 0;
  } else {
    Iter ident_pos = first;
    move_left( first, last );
    return ident_pos;
  }
}

refalrts::Iter refalrts::ident_right(
  refalrts::RefalIdentifier ident, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if ( cDataIdentifier != last->tag ) {
    return 0;
  } else if ( last->ident_info != ident ) {
    return 0;
  } else {
    Iter ident_pos = last;
    move_right( first, last );
    return ident_pos;
  }
}

bool refalrts::brackets_term(
  refalrts::Iter& res_first, refalrts::Iter& res_last,
  refalrts::Iter& pos
) {
  if (pos->tag != cDataOpenBracket) {
    return false;
  }

  refalrts::Iter right_bracket = pos->link_info;

  if (next(pos) != right_bracket) {
    res_first = next(pos);
    res_last = prev(right_bracket);
  } else {
    res_first = 0;
    res_last = 0;
  }

  return true;
}

refalrts::Iter refalrts::brackets_left(
  refalrts::Iter& res_first, refalrts::Iter& res_last,
  refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if ( cDataOpenBracket != first->tag ) {
    return 0;
  } else {
    refalrts::Iter left_bracket = first;
    refalrts::Iter right_bracket = left_bracket->link_info;

    if( next( left_bracket ) != right_bracket ) {
      res_first = next( left_bracket );
      res_last = prev( right_bracket );
    } else {
      res_first = 0;
      res_last = 0;
    }

    if( right_bracket == last ) {
      first = 0;
      last = 0;
    } else {
      first = next( right_bracket );
    }

    return left_bracket;
  }
}

refalrts::Iter refalrts::brackets_right(
  refalrts::Iter& res_first, refalrts::Iter& res_last,
  refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if( cDataCloseBracket != last->tag ) {
    return 0;
  } else {
    refalrts::Iter right_bracket = last;
    refalrts::Iter left_bracket = right_bracket->link_info;

    if( next( left_bracket ) != right_bracket ) {
      res_first = next( left_bracket );
      res_last = prev( right_bracket );
    } else {
      res_first = 0;
      res_last = 0;
    }

    if( first == left_bracket ) {
      first = 0;
      last = 0;
    } else {
      last = prev( left_bracket );
    }

    return left_bracket;
  }
}

void refalrts::bracket_pointers(
  refalrts::Iter left_bracket,
  refalrts::Iter& right_bracket)
{
  right_bracket = left_bracket->link_info;
}

bool refalrts::adt_term(
  refalrts::Iter& res_first, refalrts::Iter& res_last,
  refalrts::RefalFunctionPtr tag,
  refalrts::Iter pos
) {
  if (pos->tag != cDataOpenADT) {
    return false;
  }
  
  refalrts::Iter adt_tag = next(pos);
  
  if (adt_tag->tag != cDataFunction && adt_tag->function_info.ptr != tag) {
    return false;
  }
  
  refalrts::Iter right_bracket = pos->link_info;
  
  if (next(adt_tag) != right_bracket) {
    res_first = next(adt_tag);
    res_last = prev(right_bracket);
  } else {
    res_first = 0;
    res_last = 0;
  }
  
  return true;
}

refalrts::Iter refalrts::adt_left(
  refalrts::Iter& res_first, refalrts::Iter& res_last,
  refalrts::RefalFunctionPtr tag,
  refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if ( cDataOpenADT != first->tag ) {
    return 0;
  } else {
    refalrts::Iter left_bracket = first;
    refalrts::Iter right_bracket = left_bracket->link_info;
    refalrts::Iter pnext = next( left_bracket );

    if( pnext == right_bracket ) {
      return 0;
    } else if( cDataFunction != pnext->tag ) {
      return 0;
    } else if( pnext->function_info.ptr != tag ) {
      return 0;
    } else {
      if( next( pnext ) != right_bracket ) {
        res_first = next( pnext );
        res_last = prev( right_bracket );
      } else {
        res_first = 0;
        res_last = 0;
      }

      if( right_bracket == last ) {
        first = 0;
        last = 0;
      } else {
        first = next( right_bracket );
      }

      return left_bracket;
    }
  }
}

refalrts::Iter refalrts::adt_right(
  refalrts::Iter& res_first, refalrts::Iter& res_last,
  refalrts::RefalFunctionPtr tag,
  refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if( cDataCloseADT != last->tag ) {
    return 0;
  } else {
    refalrts::Iter right_bracket = last;
    refalrts::Iter left_bracket = right_bracket->link_info;
    refalrts::Iter pnext = next( left_bracket );

    if( pnext == right_bracket ) {
      return 0;
    } else if( cDataFunction != pnext->tag ) {
      return 0;
    } else if( pnext->function_info.ptr != tag ) {
      return 0;
    } else {
      if( next( pnext ) != right_bracket ) {
        res_first = next( pnext );
        res_last = prev( right_bracket );
      } else {
        res_first = 0;
        res_last = 0;
      }

      if( first == left_bracket ) {
        first = 0;
        last = 0;
      } else {
        last = prev( left_bracket );
      }

      return left_bracket;
    }
  }
}

void refalrts::adt_pointers(
  refalrts::Iter left_bracket,
  refalrts::Iter& tag,
  refalrts::Iter& right_bracket)
{
  refalrts::Iter pnext = next( left_bracket );
  tag = pnext;
  right_bracket = left_bracket->link_info;
}

refalrts::Iter refalrts::call_left(
  refalrts::Iter& res_first, refalrts::Iter& res_last,
  refalrts::Iter first, refalrts::Iter last
) {
  assert( (first != 0) && (last != 0) );
  assert( cDataOpenCall == first->tag );

  refalrts::Iter left_bracket = first;
  refalrts::Iter right_bracket = last;
  refalrts::Iter function = next( left_bracket );

  assert( left_bracket->link_info == right_bracket );

  if( next( function ) != right_bracket ) {
    res_first = next( function );
    res_last = prev( right_bracket );
  } else {
    res_first = 0;
    res_last = 0;
  }

  return function;
}

void refalrts::call_pointers(
  refalrts::Iter left_bracket,
  refalrts::Iter& tag,
  refalrts::Iter& right_bracket)
{
  refalrts::Iter pnext = next( left_bracket );
  tag = pnext;
  right_bracket = left_bracket->link_info;
}

bool refalrts::svar_term(
  refalrts::Iter /* svar */, refalrts::Iter pos
) {
  if (! is_open_bracket( pos )) {
    return true;
  }
  return false;
}

bool refalrts::svar_left(
  refalrts::Iter& svar, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return false;
  } else if ( is_open_bracket( first ) ) {
    return false;
  } else {
    svar = first;
    move_left( first, last );
    return true;
  }
}

bool refalrts::svar_right(
  refalrts::Iter& svar, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return false;
  } else if ( is_close_bracket( last ) ) {
    return false;
  } else {
    svar = last;
    move_right( first, last );
    return true;
  }
}

refalrts::Iter refalrts::tvar_left(
  refalrts::Iter& tvar, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if ( is_open_bracket( first ) ) {
    refalrts::Iter right_bracket = first->link_info;

    tvar = first;
    first = right_bracket;
    move_left( first, last );
    return right_bracket;
  } else {
    tvar = first;
    move_left( first, last );
    return tvar;
  }
}

refalrts::Iter refalrts::tvar_right(
  refalrts::Iter& tvar, refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  if( empty_seq( first, last ) ) {
    return 0;
  } else if ( is_close_bracket( last ) ) {
    refalrts::Iter right_bracket = last;
    refalrts::Iter left_bracket = right_bracket->link_info;

    tvar = left_bracket;
    last = left_bracket;
    move_right( first, last );
    return right_bracket;
  } else {
    tvar = last;
    move_right( first, last );
    return tvar;
  }
}

namespace refalrts {

class UnexpectedTypeException { };

} // namespace refalrts

namespace {

bool equal_nodes(
  refalrts::Iter node1, refalrts::Iter node2
) // throws refalrts::UnexpectedTypeException
{
  if( node1->tag != node2->tag ) {
    return false;
  } else {
    switch( node1->tag ) {
      case refalrts::cDataChar:
        return (node1->char_info == node2->char_info);
        // break;

      case refalrts::cDataNumber:
        return (node1->number_info == node2->number_info);
        // break;

      case refalrts::cDataFunction:
        return (node1->function_info.ptr == node2->function_info.ptr);
        // break;

      case refalrts::cDataIdentifier:
        return (node1->ident_info == node2->ident_info);
        // break;

      /*
        Сведения о связях между скобками нужны для других целей, здесь
        же нам важны только их одновременные появления.
      */
      case refalrts::cDataOpenBracket:
      case refalrts::cDataCloseBracket:
      case refalrts::cDataOpenADT:
      case refalrts::cDataCloseADT:
        return true;
        // break;

      case refalrts::cDataFile:
        return (node1->file_info == node2->file_info);
        // break;

      case refalrts::cDataClosure:
        return (node1->link_info == node2->link_info);
        // break;

      /*
        Данная функция предназначена только для использования функциями рас-
        познавания образца. Поэтому других узлов мы тут не ожидаем.
      */
      default:
        assert( SWITCH_DEFAULT_VIOLATION );
        throw refalrts::UnexpectedTypeException();
        // break;
    }
    // Все ветви в case завершаются либо return, либо throw.
  }
}

} // unnamed namespace

namespace refalrts {

namespace profiler {

void add_match_repeated_tvar_time(clock_t duration);
void add_match_repeated_evar_time(clock_t duration);

}

}

namespace {

bool equal_expressions(
  refalrts::Iter first1, refalrts::Iter last1,
  refalrts::Iter first2, refalrts::Iter last2
) // throws refalrts::UnexpectedTypeException
{
  assert( (first1 == 0) == (last1 == 0) );
  assert( (first2 == 0) == (last2 == 0) );

  clock_t start_match = clock();

  for(
    /* Пользуемся аргументами функции, инициализация не нужна */;
    // Порядок условий важен
    ! refalrts::empty_seq( first1, last1 )
      && ! refalrts::empty_seq( first2, last2 )
      && equal_nodes( first1, first2 );
    refalrts::move_left( first1, last1 ), refalrts::move_left( first2, last2 )
  ) {
    continue;
  }

  /*
    Здесь empty_seq( first1, last1 ) || empty_seq( first2, last2 )
      || !equal_nodes( first1, first2 )
  */

  refalrts::profiler::add_match_repeated_tvar_time(clock() - start_match);

  // Успешное завершение -- если мы достигли конца в обоих выражениях
  if(
    refalrts::empty_seq( first1, last1 ) && refalrts::empty_seq( first2, last2 )
  ) {
    return true;
  } else {
    // Любое другое завершение цикла свидетельствует о несовпадении
    return false;
  }
}

} // unnamed namespace

bool refalrts::repeated_stvar_term(
  refalrts::Iter stvar_sample, refalrts::Iter pos
) {
  if (svar_term(stvar_sample, stvar_sample)) {
    return svar_term(pos, pos) && equal_nodes(pos, stvar_sample);
  } else if (is_open_bracket(pos)) {
    refalrts::Iter pos_e = pos->link_info;
    refalrts::Iter stvar_sample_e = stvar_sample->link_info;
    
    return equal_expressions(
      stvar_sample, stvar_sample_e, pos, pos_e
    );
  } else {
    return false;
  }
}

refalrts::Iter refalrts::repeated_stvar_left(
  refalrts::Iter& stvar, refalrts::Iter stvar_sample,
  refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  refalrts::Iter left_term = 0;
  refalrts::Iter copy_last = last;

  if( ! is_open_bracket( stvar_sample ) && svar_left( stvar, first, last ) ) {
    if (equal_nodes( stvar, stvar_sample ))
      return stvar;
    else
      return 0;
  } else if( tvar_left( left_term, first, last ) ) {
    refalrts::Iter left_term_e;
    refalrts::Iter stvar_sample_e;

    if( empty_seq( first, last ) ) {
      left_term_e = copy_last;
    } else {
      left_term_e = prev( first );
    }

    if( is_open_bracket( stvar_sample ) ) {
      stvar_sample_e = stvar_sample->link_info;
    } else {
      stvar_sample_e = stvar_sample;
    }

    bool equal = equal_expressions(
      left_term, left_term_e,
      stvar_sample, stvar_sample_e
    );

    if( equal ) {
      stvar = left_term;

      return left_term_e;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

refalrts::Iter refalrts::repeated_stvar_right(
  refalrts::Iter& stvar, refalrts::Iter stvar_sample,
  refalrts::Iter& first, refalrts::Iter& last
) {
  assert( (first == 0) == (last == 0) );

  refalrts::Iter right_term = 0;
  refalrts::Iter old_last = last;

  if( ! is_open_bracket( stvar_sample ) && svar_right( stvar, first, last ) ) {
    if (equal_nodes( stvar, stvar_sample ))
      return stvar;
    else
      return 0;
  } else if( tvar_right( right_term, first, last ) ) {
    refalrts::Iter right_term_e = old_last;
    refalrts::Iter stvar_sample_e;

    if( is_open_bracket( stvar_sample ) ) {
      stvar_sample_e = stvar_sample->link_info;
    } else {
      stvar_sample_e = stvar_sample;
    }

    bool equal = equal_expressions(
      right_term, right_term_e,
      stvar_sample, stvar_sample_e
    );

    if( equal ) {
      stvar = right_term;

      return right_term_e;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

bool refalrts::repeated_evar_left(
  refalrts::Iter& evar_b, refalrts::Iter& evar_e,
  refalrts::Iter evar_b_sample, refalrts::Iter evar_e_sample,
  refalrts::Iter& first, refalrts::Iter& last
) {
  clock_t start_match = clock();
  refalrts::Iter current = first;
  refalrts::Iter cur_sample = evar_b_sample;
  refalrts::Iter copy_last = last;

  for(
    /* инициализация выше */;
    // порядок условий важен
    !empty_seq( current, copy_last )
      && !empty_seq( cur_sample, evar_e_sample )
      && equal_nodes( current, cur_sample );
    move_left( cur_sample, evar_e_sample ), move_left( current, copy_last )
  ) {
    continue;
  }

  refalrts::profiler::add_match_repeated_evar_time(clock() - start_match);

  /*
    Здесь empty_seq( current, copy_last )
      || empty_seq( cur_sample, evar_e_sample
      || ! equal_nodes( current, cur_sample )
  */
  if( empty_seq( cur_sample, evar_e_sample ) ) {
    // Это нормальное завершение цикла -- вся образцовая переменная проверена

    if( empty_seq( current, copy_last ) ) {
      evar_b = first;
      evar_e = last;

      first = 0;
      last = 0;
    } else {
      evar_b = first;
      evar_e = prev( current );

      first = current;
    }

    return true;
  } else {
    return false;
  }
}

bool refalrts::repeated_evar_right(
  refalrts::Iter& evar_b, refalrts::Iter& evar_e,
  refalrts::Iter evar_b_sample, refalrts::Iter evar_e_sample,
  refalrts::Iter& first, refalrts::Iter& last
) {
  clock_t start_match = clock();
  refalrts::Iter current = last;
  refalrts::Iter cur_sample = evar_e_sample;
  refalrts::Iter copy_first = first;

  for(
    /* инициализация выше */;
    // порядок перечисления условий важен
    !empty_seq( copy_first, current )
      && !empty_seq( evar_b_sample, cur_sample )
      && equal_nodes( current, cur_sample );
    move_right( copy_first, current ), move_right( evar_b_sample, cur_sample )
  ) {
    continue;
  }

  refalrts::profiler::add_match_repeated_evar_time(clock() - start_match);

  /*
    Здесь empty_seq( copy_first, current )
      || empty_seq( evar_b_sample, cur_sample )
      || ! equal_nodes( current, cur_sample )
  */

  if( empty_seq( evar_b_sample, cur_sample ) ) {
    // Это нормальное завершение цикла: вся переменная-образец просмотрена

    if( empty_seq( copy_first, current ) ) {
      evar_b = first;
      evar_e = last;

      first = 0;
      last = 0;
    } else {
      evar_b = next( current );
      evar_e = last;

      last = current;
    }

    return true;
  } else {
    return false;
  }
}

bool refalrts::open_evar_advance(
  Iter& evar_b, Iter& evar_e,
  Iter& first, Iter& last
) {
  assert( (evar_b == 0) == (evar_e == 0) );

  refalrts::Iter prev_first = 0;

  if ( tvar_left( prev_first, first, last ) ) {
    if (! evar_b) {
      evar_b = prev_first;
    }

    if ( is_open_bracket( prev_first ) ) {
      evar_e = prev_first->link_info;
    } else {
      evar_e = prev_first;
    }

    return true;
  } else {
    return false;
  }
}

unsigned refalrts::read_chars(
  char buffer[], unsigned buflen, refalrts::Iter& first, refalrts::Iter& last
) {
  unsigned read = 0;
  while (
    read != buflen && ! empty_seq(first, last)
      && first->tag == refalrts::cDataChar
  ) {
    buffer[read] = first->char_info;
    ++ read;
    move_left( first, last );
  }
  return read;
}

//------------------------------------------------------------------------------

// Операции распределителя памяти

namespace refalrts{

namespace allocator {

void reset_allocator();
bool alloc_node( Iter& node );
Iter free_ptr();
void splice_to_freelist( Iter begin, Iter end );
void splice_from_freelist( Iter pos );

} // namespace allocator

} // namespace refalrts

//------------------------------------------------------------------------------

// Средства профилирования

namespace refalrts {

namespace profiler {

extern void start_building_result();

} // namespace profiler

} // namespace refalrts

//------------------------------------------------------------------------------

// Операции построения результата

void refalrts::reset_allocator() {
  profiler::start_building_result();
  allocator::reset_allocator();
}

namespace {

bool copy_node( refalrts::Iter& res, refalrts::Iter sample ) {
  switch( sample->tag ) {
    case refalrts::cDataChar:
      return refalrts::alloc_char( res, sample->char_info );
      // break;

    case refalrts::cDataNumber:
      return refalrts::alloc_number( res, sample->number_info );
      // break;

    case refalrts::cDataFunction:
      return refalrts::alloc_name(
        res, sample->function_info.ptr, sample->function_info.name
      );
      // break;

    case refalrts::cDataIdentifier:
      return refalrts::alloc_ident( res, sample->ident_info );
      //break;

    case refalrts::cDataOpenBracket:
      return refalrts::alloc_open_bracket( res );
      // break;

    case refalrts::cDataCloseBracket:
      return refalrts::alloc_close_bracket( res );
      // break;

    case refalrts::cDataOpenADT:
      return refalrts::alloc_open_adt( res );
      // break;

    case refalrts::cDataCloseADT:
      return refalrts::alloc_close_adt( res );
      // break;

    case refalrts::cDataClosure: {
      bool allocated = refalrts::allocator::alloc_node( res );
      if( allocated ) {
        res->tag = refalrts::cDataClosure;
        refalrts::Iter head = sample->link_info;
        res->link_info = head;
        ++ (head->number_info);
        return true;
      } else {
        return false;
      }
    }
    // break;

    case refalrts::cDataFile: {
      bool allocated = refalrts::allocator::alloc_node( res );
      if( allocated ) {
        res->tag = refalrts::cDataFile;
        res->file_info = sample->file_info;
        return true;
      } else {
        return false;
      }
    }
    // break;

    /*
      Копируем только объектное выражение -- никаких вызовов функций
      быть не должно.
    */
    default:
      assert( SWITCH_DEFAULT_VIOLATION );
      throw refalrts::UnexpectedTypeException();
      // break;
  }
}

} // unnamed namespace


namespace refalrts {

namespace vm {

void make_dump( refalrts::Iter begin, refalrts::Iter end );

} // namespace vm

namespace profiler {

void add_copy_tevar_time(clock_t duration);

} // namespace profiler

} // namespace refalrts

namespace {

bool copy_nonempty_evar(
  refalrts::Iter& evar_res_b, refalrts::Iter& evar_res_e,
  refalrts::Iter evar_b_sample, refalrts::Iter evar_e_sample
) {
  clock_t start_copy_time = clock();

  refalrts::Iter res = 0;
  refalrts::Iter bracket_stack = 0;

  refalrts::Iter prev_res_begin =
    prev( refalrts::allocator::free_ptr() );

  while( ! refalrts::empty_seq( evar_b_sample, evar_e_sample ) ) {
    if( ! copy_node( res, evar_b_sample ) ) {
      return false;
    }

    if( is_open_bracket( res ) ) {
      res->link_info = bracket_stack;
      bracket_stack = res;
    } else if( is_close_bracket( res ) ) {
      assert( bracket_stack != 0 );

      refalrts::Iter open_cobracket = bracket_stack;
      bracket_stack = bracket_stack->link_info;
      refalrts::link_brackets( open_cobracket, res );
    }

    refalrts::move_left( evar_b_sample, evar_e_sample );
  }

  assert( bracket_stack == 0 );

  evar_res_b = next( prev_res_begin );
  evar_res_e = res;

  refalrts::profiler::add_copy_tevar_time(clock() - start_copy_time);

  return true;
}

} // unnamed namespace

bool refalrts::copy_evar(
  refalrts::Iter& evar_res_b, refalrts::Iter& evar_res_e,
  refalrts::Iter evar_b_sample, refalrts::Iter evar_e_sample
) {
  if( empty_seq( evar_b_sample, evar_e_sample ) ) {
    evar_res_b = 0;
    evar_res_e = 0;
    return true;
  } else {
    return copy_nonempty_evar(
      evar_res_b, evar_res_e, evar_b_sample, evar_e_sample
    );
  }
}

bool refalrts::copy_stvar(
  refalrts::Iter& stvar_res, refalrts::Iter stvar_sample
) {
  if( is_open_bracket( stvar_sample ) ) {
    refalrts::Iter end_of_sample = stvar_sample->link_info;
    refalrts::Iter end_of_res;
    return copy_evar(
      stvar_res, end_of_res, stvar_sample, end_of_sample
    );
  } else {
    return copy_node(stvar_res, stvar_sample);
  }
}

bool refalrts::alloc_copy_evar(
  refalrts::Iter& res,
  refalrts::Iter evar_b_sample, refalrts::Iter evar_e_sample
) {
  if( empty_seq( evar_b_sample, evar_e_sample ) ) {
    res = 0;
    return true;
  } else {
    refalrts::Iter res_e = 0;
    return copy_nonempty_evar(
      res, res_e, evar_b_sample, evar_e_sample
    );
  }
}

bool refalrts::alloc_copy_svar_(
  refalrts::Iter& svar_res, refalrts::Iter svar_sample
) {
  return copy_node( svar_res, svar_sample );
}


bool refalrts::alloc_char( refalrts::Iter& res, char ch ) {
  if( allocator::alloc_node( res ) ) {
    res->tag = cDataChar;
    res->char_info = ch;
    return true;
  } else {
    return false;
  }
}

bool refalrts::alloc_number(
  refalrts::Iter& res, refalrts::RefalNumber num
) {
  if( allocator::alloc_node( res ) ) {
    res->tag = cDataNumber;
    res->number_info = num;
    return true;
  } else {
    return false;
  }
}

#ifdef MODULE_REFAL
const char *unknown() { return "@unknown"; }
#else
const char *unknown = "@unknown";
#endif

bool refalrts::alloc_name(
  refalrts::Iter& res,
  refalrts::RefalFunctionPtr fn,
  refalrts::RefalFuncName name
) {
  if( allocator::alloc_node( res ) ) {
    res->tag = cDataFunction;
    res->function_info.ptr = fn;
    if( name != 0 ) {
      res->function_info.name = name;
    } else {
      res->function_info.name = unknown;
    }
    return true;
  } else {
    return false;
  }
}

bool refalrts::alloc_ident(
  refalrts::Iter& res, refalrts::RefalIdentifier ident
) {
  if( allocator::alloc_node( res ) ) {
    res->tag = cDataIdentifier;
    res->ident_info = ident;
    return true;
  } else {
    return false;
  }
}

namespace {

bool alloc_some_bracket( refalrts::Iter& res, refalrts::DataTag tag ) {
  if( refalrts::allocator::alloc_node( res ) ) {
    res->tag = tag;
    return true;
  } else {
    return false;
  }
}

void link_adjacent( refalrts::Iter left, refalrts::Iter right ) {
  if( left != 0 ) {
    left->next = right;
  }

  if( right != 0 ) {
    right->prev = left;
  }
}

bool alloc_closure( refalrts::Iter& res ) {
  bool allocated = refalrts::allocator::alloc_node( res );
  if( allocated ) {
    refalrts::Iter head = 0;
    allocated = refalrts::allocator::alloc_node( head );
    if( allocated ) {
      refalrts::Iter prev_head = prev( head );
      refalrts::Iter next_head = next( head );

      link_adjacent( prev_head, next_head );
      link_adjacent( head, head );

      res->tag = refalrts::cDataClosure;
      res->link_info = head;

      head->tag = refalrts::cDataClosureHead;
      head->number_info = 1;

      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

} // unnamed namespace

bool refalrts::alloc_open_adt( refalrts::Iter& res ) {
  return alloc_some_bracket( res, cDataOpenADT );
}

bool refalrts::alloc_close_adt( refalrts::Iter& res ) {
  return alloc_some_bracket( res, cDataCloseADT );
}

bool refalrts::alloc_open_bracket( refalrts::Iter& res ) {
  return alloc_some_bracket( res, cDataOpenBracket );
}

bool refalrts::alloc_close_bracket( refalrts::Iter& res ) {
  return alloc_some_bracket( res, cDataCloseBracket );
}

bool refalrts::alloc_open_call( refalrts::Iter& res ) {
  return alloc_some_bracket( res, cDataOpenCall );
}

bool refalrts::alloc_close_call( refalrts::Iter& res ) {
  return alloc_some_bracket( res, cDataCloseCall );
}

bool refalrts::alloc_chars(
  refalrts::Iter& res_b, refalrts::Iter& res_e,
  const char buffer[], unsigned buflen
) {
  if( buflen == 0 ) {
    res_b = 0;
    res_e = 0;
    return true;
  } else {
    refalrts::Iter before_begin_seq = prev( refalrts::allocator::free_ptr() );
    refalrts::Iter end_seq = 0;

    for( unsigned i = 0; i < buflen; ++ i ) {
      if( ! alloc_char( end_seq, buffer[i] ) ) {
        return false;
      }
    }

    res_b = next( before_begin_seq );
    res_e = end_seq;

    return true;
  }
}

bool refalrts::alloc_string(
  refalrts::Iter& res_b, refalrts::Iter& res_e, const char *string
) {
  if( *string == '\0' ) {
    res_b = 0;
    res_e = 0;
    return true;
  } else {
    refalrts::Iter before_begin_seq = prev( refalrts::allocator::free_ptr() );
    refalrts::Iter end_seq = 0;

    for( const char *p = string; *p != '\0'; ++ p ) {
      if( ! alloc_char( end_seq, *p ) ) {
        return false;
      }
    }

    res_b = next( before_begin_seq );
    res_e = end_seq;

    return true;
  }
}

namespace refalrts {

namespace vm {

void push_stack( refalrts::Iter call_bracket );

} // namespace vm

} // namespace refalrts

void refalrts::push_stack( Iter call_bracket ) {
  vm::push_stack( call_bracket );
}

void refalrts::link_brackets( Iter left, Iter right ) {
  left->link_info = right;
  right->link_info = left;
}

namespace {

refalrts::Iter list_splice(
  refalrts::Iter res, refalrts::Iter begin, refalrts::Iter end
) {

  VALID_LINKED( res );
  VALID_LINKED( res->prev );
  VALID_LINKED( begin );
  VALID_LINKED( begin->prev );
  VALID_LINKED( end );
  VALID_LINKED( end->prev );

  if( (res == begin) || refalrts::empty_seq( begin, end ) ) {
    // Цель достигнута сама по себе
    return res;
  } else if ( res == next(end) ) {
    // Цель достигнута сама по себе
    return begin;
  } else {
    refalrts::Iter prev_res = prev( res );
    refalrts::Iter prev_begin = prev( begin );
    refalrts::Iter next_end = next( end );

    link_adjacent( prev_res, begin );
    link_adjacent( end, res );
    link_adjacent( prev_begin, next_end );
  }

  VALID_LINKED( res );
  VALID_LINKED( res->prev );
  VALID_LINKED( begin );
  VALID_LINKED( begin->prev );
  VALID_LINKED( end );
  VALID_LINKED( end->next );

  return begin;
}

} // unnamed namespace

void refalrts::reinit_svar(refalrts::Iter res, refalrts::Iter sample) {
  res->tag = sample->tag;

  switch( sample->tag ) {
    case refalrts::cDataChar:
      res->char_info = sample->char_info;
      break;

    case refalrts::cDataNumber:
      res->number_info = sample->number_info;
      break;

    case refalrts::cDataFunction:
      res->function_info = sample->function_info;
      break;

    case refalrts::cDataIdentifier:
      res->ident_info = sample->ident_info;
      break;

    case refalrts::cDataClosure: {
      res->tag = refalrts::cDataClosure;
      refalrts::Iter head = sample->link_info;
      res->link_info = head;
      ++ (head->number_info);
    }
    break;

    case refalrts::cDataFile:
      res->file_info = sample->file_info;
      break;

    /*
      Копируем только атом, скобок быть не должно.
    */
    default:
      assert( SWITCH_DEFAULT_VIOLATION );
      throw refalrts::UnexpectedTypeException();
  }
}

void refalrts::reinit_char(refalrts::Iter res, char ch) {
  res->tag = cDataChar;
  res->char_info  = ch;
}

void refalrts::update_char(refalrts::Iter res, char ch) {
  res->char_info  = ch;
}

void refalrts::reinit_number(
  refalrts::Iter res, refalrts::RefalNumber num)
{
  res->tag = cDataNumber;
  res->number_info  = num;
}

void refalrts::update_number(
  refalrts::Iter res, refalrts::RefalNumber num
) {
  res->number_info  = num;
}


void refalrts::reinit_name(
  refalrts::Iter res,
  refalrts::RefalFunctionPtr func,
  refalrts::RefalFuncName name
) {
  res->tag = cDataFunction;
  res->function_info.ptr  = func;
  res->function_info.name  = name;
}

void refalrts::update_name(
  refalrts::Iter res,
  refalrts::RefalFunctionPtr func,
  refalrts::RefalFuncName name
) {
  res->function_info.ptr  = func;
  res->function_info.name  = name;
}

void refalrts::reinit_ident(
  refalrts::Iter res, refalrts::RefalIdentifier ident
) {
  res->tag = cDataIdentifier;
  res->ident_info  = ident;
}

void refalrts::update_ident(
  refalrts::Iter res, refalrts::RefalIdentifier ident
) {
  res->ident_info  = ident;
}

void refalrts::reinit_open_bracket(
  refalrts::Iter res
) {
  res->tag = cDataOpenBracket;
}

void refalrts::reinit_close_bracket(
  refalrts::Iter res
) {
  res->tag = cDataCloseBracket;
}

void refalrts::reinit_open_adt(
  refalrts::Iter res
) {
  res->tag = cDataOpenADT;
}

void refalrts::reinit_close_adt(
  refalrts::Iter res
) {
  res->tag = cDataCloseADT;
}

void refalrts::reinit_open_call(
  refalrts::Iter res
) {
  res->tag = cDataOpenCall;
}


void refalrts::reinit_close_call(
  refalrts::Iter res
) {
  res->tag = cDataCloseCall;
}




refalrts::Iter refalrts::splice_elem(
  refalrts::Iter res, refalrts::Iter elem
) {
  return list_splice( res, elem, elem );
}

refalrts::Iter refalrts::splice_stvar(
  refalrts::Iter res, refalrts::Iter var
) {
  refalrts::Iter var_end;
  if( is_open_bracket( var ) ) {
    var_end = var->link_info;
  } else {
    var_end = var;
  }

  return list_splice( res, var, var_end );
}

refalrts::Iter refalrts::splice_evar(
  refalrts::Iter res, refalrts::Iter begin, refalrts::Iter end
) {
  return list_splice( res, begin, end );
}

void refalrts::splice_to_freelist( refalrts::Iter begin, refalrts::Iter end ) {
  allocator::splice_to_freelist( begin, end );
}

extern void refalrts::splice_to_freelist_open(
  refalrts::Iter before_first, refalrts::Iter after_last
) {
  if (before_first->next != after_last) {
    refalrts::splice_to_freelist(before_first->next, after_last->prev);
  }
}

void refalrts::splice_from_freelist( refalrts::Iter pos ) {
  allocator::splice_from_freelist( pos );
}

refalrts::FnResult refalrts::create_closure(
  refalrts::Iter begin, refalrts::Iter end
) {
  refalrts::Iter closure_b = begin;
  refalrts::Iter closure_e = end;

  refalrts::move_left( closure_b, closure_e ); // пропуск <
  refalrts::move_left( closure_b, closure_e ); // пропуск имени функции
  refalrts::move_right( closure_b, closure_e ); // пропуск >

  if( empty_seq( closure_b, closure_e ) )
    return refalrts::cRecognitionImpossible;

  refalrts::Iter closure = 0;

  if( ! alloc_closure( closure ) )
    return refalrts::cNoMemory;

  refalrts::Iter head = closure->link_info;

  list_splice( head, closure_b, closure_e );
  list_splice( begin, closure, closure );
  refalrts::splice_to_freelist( begin, end );

  return refalrts::cSuccess;
}

/*
  Собственно замыкание (функция + контекст) определяется как
  [next(head), prev(head)]. Т.к. замыкание создаётся только функцией
  create_closure, которая гарантирует непустоту замыкания, то
  next(head) != head, prev(head) != head.
*/

// Развернуть замыкание
refalrts::Iter refalrts::unwrap_closure( refalrts::Iter closure ) {
  assert( closure->tag == refalrts::cDataClosure );

  refalrts::Iter before_closure = prev( closure );
  refalrts::Iter head = closure->link_info;
  refalrts::Iter end_of_closure = prev( head );

  assert( head != prev( head ) );
  assert( head != next( head ) );

  link_adjacent( before_closure, head );
  link_adjacent( end_of_closure, closure );

  closure->tag = refalrts::cDataUnwrappedClosure;

  return prev(head);
}

// Свернуть замыкание
refalrts::Iter refalrts::wrap_closure( refalrts::Iter closure ) {
  assert( closure->tag == refalrts::cDataUnwrappedClosure );

  refalrts::Iter head = closure->link_info;
  refalrts::Iter before_closure = prev( head );
  refalrts::Iter end_of_closure = prev( closure );

  assert( head != prev( head ) );
  assert( head != next( head ) );

  link_adjacent( before_closure, closure );
  link_adjacent( end_of_closure, head );

  closure->tag = refalrts::cDataClosure;

  return next(closure);
}

//------------------------------------------------------------------------------

// Инициализация головного узла статического ящика

namespace refalrts {

namespace vm {

extern NodePtr g_left_swap_ptr;

} // namespace vm

} // namespace refalrts

refalrts::Iter refalrts::initialize_swap_head( refalrts::Iter head ) {
  assert( cDataFunction == head->tag );

  splice_elem( vm::g_left_swap_ptr, head );
  refalrts::RefalFuncName name = head->function_info.name;
  head->tag = cDataSwapHead;
  head->swap_info.next_head = vm::g_left_swap_ptr;
  head->swap_info.name = name;
  vm::g_left_swap_ptr = head;
  return vm::g_left_swap_ptr;
}

void refalrts::swap_info_bounds(
  refalrts::Iter& first, refalrts::Iter& last, refalrts::Iter head
) {
  assert( cDataSwapHead == head->tag );

  first = head;
  last = head->swap_info.next_head;
  move_left( first, last );
  move_right( first, last );
}

void refalrts::swap_save(
  refalrts::Iter head, refalrts::Iter first, refalrts::Iter last
) {
  assert( cDataSwapHead == head->tag );

  list_splice( head->swap_info.next_head, first, last );
}

//------------------------------------------------------------------------------

// Средства профилирования

namespace refalrts {

namespace profiler {

extern void start_generated_function();
extern void read_counters(unsigned long counters[]);
extern void start_e_loop();
extern void stop_e_loop();

} // namespace profiler

} // namespace refalrts

void refalrts::this_is_generated_function() {
  refalrts::profiler::start_generated_function();
}

unsigned long refalrts::ticks_per_second() {
  return CLOCKS_PER_SEC;
}

void refalrts::read_performance_counters(unsigned long counters[]) {
  refalrts::profiler::read_counters(counters);
}

void refalrts::start_sentence() {
  refalrts::profiler::stop_e_loop();
}

void refalrts::start_e_loop() {
  refalrts::profiler::start_e_loop();
}

//------------------------------------------------------------------------------

// Прочие операции

namespace refalrts {

namespace vm {

extern int g_ret_code;
extern void print_seq(FILE *output, refalrts::Iter begin, refalrts::Iter end);

} // namespace vm

} // namespace refalrts

void refalrts::set_return_code( int code ) {
  refalrts::vm::g_ret_code = code;
}

void refalrts::debug_print_expr(
  void *file, refalrts::Iter first, refalrts::Iter last
) {
  refalrts::vm::print_seq( static_cast<FILE*>(file), first, last );
}

//==============================================================================
// Распределитель памяти
//==============================================================================

namespace refalrts {

namespace allocator {

bool create_nodes();

void free_memory();

extern refalrts::Node g_last_marker;

refalrts::Node g_first_marker =
  { 0, & g_last_marker, refalrts::cDataIllegal, { '\0' } };
refalrts::Node g_last_marker =
  { & g_first_marker, 0, refalrts::cDataIllegal, { '\0' } };

refalrts::NodePtr g_free_ptr = & g_last_marker;

namespace pool {

enum { cChunkSize = 1000 };
typedef struct Chunk {
  Chunk *next;
  refalrts::Node elems[cChunkSize];
} Chunk;

typedef Chunk *ChunkPtr;

refalrts::NodePtr alloc_node();
void free();
bool grow();

ChunkPtr g_pool = 0;
unsigned g_avail = 0;
refalrts::Node *g_pnext_node = 0;

} // namespace pool

unsigned g_memory_use = 0;

} // namespace allocator

} // namespace refalrts

inline void refalrts::allocator::reset_allocator() {
  g_free_ptr = g_first_marker.next;
}

bool refalrts::allocator::alloc_node( refalrts::Iter& node ) {
  if( (g_free_ptr == & g_last_marker) && ! create_nodes() ) {
    return false;
  } else {
    if( refalrts::cDataClosure == g_free_ptr->tag ) {
      refalrts::Iter head = g_free_ptr->link_info;
      -- head->number_info;

      if( 0 == head->number_info ) {
        unwrap_closure( g_free_ptr );
        // теперь перед g_free_ptr находится "развёрнутое" замыкание
        g_free_ptr->tag = refalrts::cDataClosureHead;
        g_free_ptr->number_info = 407193; // :-)

        g_free_ptr = head;
      }
    }

    node = g_free_ptr;
    g_free_ptr = next( g_free_ptr );
    node->tag = refalrts::cDataIllegal;
    return true;
  }
}

refalrts::Iter refalrts::allocator::free_ptr() {
  return g_free_ptr;
}

void refalrts::allocator::splice_to_freelist(
  refalrts::Iter begin, refalrts::Iter end
) {
  reset_allocator();
  g_free_ptr = list_splice( g_free_ptr, begin, end );
}

void refalrts::allocator::splice_from_freelist( refalrts::Iter pos ) {
  if (g_free_ptr != g_first_marker.next ) {
    list_splice( pos, g_first_marker.next, g_free_ptr->prev );
  }
}

bool refalrts::allocator::create_nodes() {
  refalrts::NodePtr new_node = refalrts::allocator::pool::alloc_node();

#ifdef MEMORY_LIMIT

  if( g_memory_use >= MEMORY_LIMIT ) {
    return false;
  }

#endif //ifdef MEMORY_LIMIT

  if( new_node == 0 ) {
    return false;
  } else {
    refalrts::NodePtr before_free_ptr = prev( g_free_ptr );
    before_free_ptr->next = new_node;
    new_node->prev = before_free_ptr;

    g_free_ptr->prev = new_node;
    new_node->next = g_free_ptr;

    g_free_ptr = new_node;
    g_free_ptr->tag = refalrts::cDataIllegal;
    ++ g_memory_use;

    return true;
  }
}

void refalrts::allocator::free_memory() {
  refalrts::allocator::pool::free();
#ifndef DONT_PRINT_STATISTICS
  fprintf(
    stderr,
    "Memory used %d nodes, %d * %lu = %lu bytes\n",
    g_memory_use,
    g_memory_use,
    static_cast<unsigned long>(sizeof(Node)),
    static_cast<unsigned long>(g_memory_use * sizeof(Node))
  );
#endif // DONT_PRINT_STATISTICS
}

refalrts::NodePtr refalrts::allocator::pool::alloc_node() {
  if( (g_avail != 0) || grow() ) {
    -- g_avail;
    return g_pnext_node++;
  } else {
    return 0;
  }
}

bool refalrts::allocator::pool::grow() {
  ChunkPtr p = static_cast<ChunkPtr>( malloc( sizeof(Chunk) ) );
  if( p != 0 ) {
    p->next = g_pool;
    g_pool = p;
    g_avail = cChunkSize;
    g_pnext_node = p->elems;
    return true;
  } else {
    return false;
  }
}

void refalrts::allocator::pool::free() {
  while( g_pool != 0 ) {
    ChunkPtr p = g_pool;
    g_pool = g_pool->next;
    ::free( p );
  }
}

//==============================================================================
// Упрощённый профилировщик
//==============================================================================

namespace refalrts {

namespace profiler {

clock_t g_start_program_time;
clock_t g_start_pattern_match_time;
clock_t g_total_pattern_match_time;
clock_t g_start_building_result_time;
clock_t g_total_building_result_time;
clock_t g_total_copy_tevar_time;
clock_t g_total_match_repeated_tvar_time;
clock_t g_total_match_repeated_evar_time;
clock_t g_start_e_loop;
clock_t g_total_e_loop;
clock_t g_total_match_repeated_tvar_time_outside_e;
clock_t g_total_match_repeated_evar_time_outside_e;

bool g_in_generated;
int g_in_e_loop;

void start_profiler();
void end_profiler();
void start_generated_function();
void after_step();
void read_counters(unsigned long counters[]);
void add_copy_tevar_time(clock_t duration);
void add_match_repeated_tvar_time(clock_t duration);
void add_match_repeated_evar_time(clock_t duration);
void start_e_loop();
void stop_e_loop();

#ifndef DONT_PRINT_STATISTICS
struct TimeItem {
  const char *name;
  unsigned long counter;
};

int reverse_compare(const void *left_void, const void *right_void);
#endif // DONT_PRINT_STATISTICS

} // namespace profiler

} // namespace refalrts

void refalrts::profiler::start_profiler() {
  g_start_program_time = clock();
  g_in_generated = false;
}

#ifndef DONT_PRINT_STATISTICS

int refalrts::profiler::reverse_compare(
  const void *left_void, const void *right_void
) {
  const TimeItem *left = static_cast<const TimeItem *>(left_void);
  const TimeItem *right = static_cast<const TimeItem *>(right_void);

  if (left->counter > right->counter) {
    return -1;
  } else if (left->counter < right->counter) {
    return +1;
  } else {
    return 0;
  }
}

#endif // DONT_PRINT_STATISTICS

void refalrts::profiler::end_profiler() {
  refalrts::profiler::after_step();
#ifndef DONT_PRINT_STATISTICS

  unsigned long counters[cPerformanceCounter_COUNTERS_NUMBER];
  read_counters(counters);

  TimeItem items[] = {
    { "\nTotal program time", cPerformanceCounter_TotalTime },
    { "Builtin time", cPerformanceCounter_BuiltInTime },
    { "(Total refal time)", cPerformanceCounter_RefalTime },
    { "Linear pattern time", cPerformanceCounter_LinearPatternTime },
    { "Linear result time", cPerformanceCounter_LinearResultTime },
    { "Open e-loop time (clear)", cPerformanceCounter_OpenELoopTimeClear },
    {
      "Repeated e-var match time (inside e-loops)",
      cPerformanceCounter_RepeatEvarMatchTime
    },
    {
      "Repeated e-var match time (outside e-loops)",
      cPerformanceCounter_RepeatEvarMatchTimeOutsideECycle
    },
    {
      "Repeated t-var match time (inside e-loops)",
      cPerformanceCounter_RepeatTvarMatchTime
    },
    {
      "Repeated t-var match time (outside e-loops)",
      cPerformanceCounter_RepeatTvarMatchTimeOutsideECycle
    },
    { "t- and e-var copy time", cPerformanceCounter_TEvarCopyTime }
  };

  enum { nItems = sizeof(items) / sizeof(items[0]) };

  for (size_t i = 0; i < nItems; ++i) {
    items[i].counter = counters[items[i].counter];
  }

  qsort(items, nItems, sizeof(items[0]), reverse_compare);

  const double cfSECS_PER_CLOCK = 1.0 / CLOCKS_PER_SEC;
  unsigned long total = counters[refalrts::cPerformanceCounter_TotalTime];

  for (size_t i = 0; i < nItems; ++i) {
    unsigned long value = items[i].counter;

    if (value > 0) {
      double percent = (total != 0) ? 100.0 * value / total : 0.0;
      fprintf(
        stderr, "%s: %.3f seconds (%.1f %%).\n",
        items[i].name, value * cfSECS_PER_CLOCK, percent
      );
    }
  }

#endif // DONT_PRINT_STATISTICS
}

void refalrts::profiler::start_generated_function() {
  g_start_pattern_match_time = clock();
  g_in_generated = true;
}

void refalrts::profiler::start_building_result() {
  if( g_in_generated ) {
    g_start_building_result_time = clock();
    clock_t pattern_match =
      g_start_building_result_time - g_start_pattern_match_time;
    g_total_pattern_match_time += pattern_match;

    stop_e_loop();
  }
}

void refalrts::profiler::after_step() {
  if( g_in_generated ) {
    clock_t building_result = clock() - g_start_building_result_time;
    g_total_building_result_time += building_result;
  }

  assert(g_in_e_loop == 0);

  g_in_generated = false;
  g_in_e_loop = 0;
}

namespace refalrts {

namespace vm {

extern unsigned g_step_counter;

} // namespace vm

} // namespace refalrts

void refalrts::profiler::read_counters(unsigned long counters[]) {
  clock_t full_time = clock() - g_start_program_time;
  clock_t refal_time =
    g_total_pattern_match_time + g_total_building_result_time;
  clock_t pattern_time = g_total_pattern_match_time;
  clock_t result_time = g_total_building_result_time;
  counters[cPerformanceCounter_TotalTime] = full_time;
  counters[cPerformanceCounter_BuiltInTime] = full_time - refal_time;
  counters[cPerformanceCounter_RefalTime] = refal_time;
  counters[cPerformanceCounter_PatternMatchTime] = pattern_time;
  counters[cPerformanceCounter_BuildResultTime] = result_time;
  counters[cPerformanceCounter_TotalSteps] = ::refalrts::vm::g_step_counter;
  counters[cPerformanceCounter_HeapSize] =
    static_cast<unsigned long>(
      ::refalrts::allocator::g_memory_use * sizeof(Node)
    );
  counters[cPerformanceCounter_TEvarCopyTime] = g_total_copy_tevar_time;
  refal_time -= g_total_copy_tevar_time;
  result_time -= g_total_copy_tevar_time;
  counters[cPerformanceCounter_RepeatTvarMatchTime] =
    g_total_match_repeated_tvar_time;
  counters[cPerformanceCounter_RepeatTvarMatchTimeOutsideECycle] =
    g_total_match_repeated_tvar_time_outside_e;
  refal_time -= g_total_match_repeated_tvar_time_outside_e;
  pattern_time -= g_total_match_repeated_tvar_time_outside_e;
  counters[cPerformanceCounter_RepeatEvarMatchTime] =
    g_total_match_repeated_evar_time;
  counters[cPerformanceCounter_RepeatEvarMatchTimeOutsideECycle] =
    g_total_match_repeated_evar_time_outside_e;
  refal_time -= g_total_match_repeated_evar_time_outside_e;
  pattern_time -= g_total_match_repeated_evar_time_outside_e;
  counters[cPerformanceCounter_OpenELoopTime] = g_total_e_loop;
  refal_time -= g_total_e_loop;
  pattern_time -= g_total_e_loop;
  counters[cPerformanceCounter_OpenELoopTimeClear] = g_total_e_loop
    - (g_total_match_repeated_tvar_time + g_total_match_repeated_evar_time);
  counters[cPerformanceCounter_LinearRefalTime] = refal_time;
  counters[cPerformanceCounter_LinearPatternTime] = pattern_time;
  counters[cPerformanceCounter_LinearResultTime] = result_time;
}

void refalrts::profiler::add_copy_tevar_time(clock_t duration) {
  g_total_copy_tevar_time += duration;
}

void refalrts::profiler::add_match_repeated_tvar_time(clock_t duration) {
  if (g_in_e_loop) {
    g_total_match_repeated_tvar_time += duration;
  } else {
    g_total_match_repeated_tvar_time_outside_e += duration;
  }
}

void refalrts::profiler::add_match_repeated_evar_time(clock_t duration) {
  if (g_in_e_loop) {
    g_total_match_repeated_evar_time += duration;
  } else {
    g_total_match_repeated_evar_time_outside_e += duration;
  }
}

void refalrts::profiler::start_e_loop() {
  if (g_in_e_loop++ == 0) {
    g_start_e_loop = clock();
  }
}

void refalrts::profiler::stop_e_loop() {
  if (g_in_e_loop > 0) {
    g_total_e_loop += (clock() - g_start_e_loop);
    g_in_e_loop = 0;
  }
}

//==============================================================================
// Виртуальная машина
//==============================================================================

#ifdef MODULE_REFAL
#define GO_START_FUNCTION Entry_Go
#else
#define GO_START_FUNCTION Go
#endif

extern refalrts::FnResult GO_START_FUNCTION(
  refalrts::Iter, refalrts::Iter
);

namespace refalrts {

namespace vm {

void push_stack( refalrts::Iter call_bracket );
refalrts::Iter pop_stack();
bool empty_stack();

bool init_view_field();

refalrts::FnResult main_loop();
refalrts::FnResult execute_active( refalrts::Iter begin, refalrts::Iter end );
void make_dump( refalrts::Iter begin, refalrts::Iter end );
FILE* dump_stream();

void free_view_field();

refalrts::NodePtr g_stack_ptr = 0;

extern refalrts::Node g_last_marker;

refalrts::Node g_first_marker =
  { 0, & g_last_marker, refalrts::cDataIllegal, { '\0' } };
refalrts::Node g_last_marker =
  { & g_first_marker, 0, refalrts::cDataIllegal, { '\0' } };

refalrts::Iter g_begin_view_field = & g_last_marker;
const refalrts::Iter g_end_view_field = & g_last_marker;

refalrts::NodePtr g_left_swap_ptr = g_end_view_field;

unsigned g_step_counter = 0;

int g_ret_code;

} // namespace vm

} // namespace refalrts

void refalrts::vm::push_stack( refalrts::Iter call_bracket ) {
  call_bracket->link_info = g_stack_ptr;
  g_stack_ptr = call_bracket;
}

refalrts::Iter refalrts::vm::pop_stack() {
  refalrts::Iter res = g_stack_ptr;
  g_stack_ptr = g_stack_ptr->link_info;
  return res;
}

bool refalrts::vm::empty_stack() {
  return (g_stack_ptr == 0);
}

#ifdef MODULE_REFAL
//$LABEL Go
template <typename T>
struct GoL_ {
  static const char *name() {
    return "Go";
  }
};
#define GO_NAME GoL_<int>::name
#else
#define GO_NAME "Go"
#endif

bool refalrts::vm::init_view_field() {
  refalrts::reset_allocator();
  refalrts::Iter res = g_begin_view_field;
  refalrts::Iter n0 = 0;
  if( ! refalrts::alloc_open_call( n0 ) )
    return false;
  refalrts::Iter n1 = 0;
  if( ! refalrts::alloc_name( n1, GO_START_FUNCTION, GO_NAME ) )
    return false;
  refalrts::Iter n2 = 0;
  if( ! refalrts::alloc_close_call( n2 ) )
    return false;
  refalrts::push_stack( n2 );
  refalrts::push_stack( n0 );
  res = refalrts::splice_elem( res, n2 );
  res = refalrts::splice_elem( res, n1 );
  res = refalrts::splice_elem( res, n0 );
  g_begin_view_field = res;

  return true;
}

refalrts::FnResult refalrts::vm::main_loop() {
  FnResult res = cSuccess;
  while( ! empty_stack() ) {
    refalrts::Iter active_begin = pop_stack();
    assert( ! empty_stack() );
    refalrts::Iter active_end = pop_stack();

    res = execute_active( active_begin, active_end );

    ++ g_step_counter;

    if( res != cSuccess ) {
      switch( res ) {
        case refalrts::cRecognitionImpossible:
          fprintf(stderr, "\nRECOGNITION IMPOSSIBLE\n\n");
          break;

        case refalrts::cNoMemory:
          fprintf(stderr, "\nNO MEMORY\n\n");
          break;

        case refalrts::cExit:
          return res;

        default:
          fprintf(stderr, "\nUNKNOWN ERROR\n\n");
          break;
      }
      make_dump( active_begin, active_end );
      return res;
    }

    refalrts::profiler::after_step();
  }

  return res;
}

refalrts::FnResult refalrts::vm::execute_active(
  refalrts::Iter begin, refalrts::Iter end
) {

#if SHOW_DEBUG

  if( g_step_counter >= (unsigned) SHOW_DEBUG ) {
    make_dump( begin, end );
  }

#endif // SHOW_DEBUG

  refalrts::Iter function = next( begin );
  if( cDataFunction == function->tag ) {
    return refalrts::FnResult(
      (function->function_info.ptr)( begin, end ) & 0xFFU
    );
  } else if( cDataClosure == function->tag ) {
    refalrts::Iter head = function->link_info;

    if( 1 == head->number_info ) {
      /*
        Пользуемся тем, что при развёртке содержимое замыкания оказывается
        в поле зрения между головой и (развёрнутым!) узлом замыкания.
        Во избежание проблем, связанным с помещением развёрнутого замыкания
        в список свободных блоков, проинициализируем его как голову замыкания.
      */
      unwrap_closure( function );
      function->tag = cDataClosureHead;
      function->number_info = 73501505; // :-)
      splice_to_freelist( function, function );
      splice_to_freelist( head, head );
    } else {
      refalrts::Iter begin_argument = next( function );
      refalrts::Iter closure_b = 0;
      refalrts::Iter closure_e = 0;

      if( ! copy_evar( closure_b, closure_e, next(head), prev(head) ) )
        return cNoMemory;

      list_splice( begin_argument, closure_b, closure_e );
      splice_to_freelist( function, function );
    }

    refalrts::vm::push_stack( end );
    refalrts::vm::push_stack( begin );

    return cSuccess;
  } else {
    return cRecognitionImpossible;
  }
}

namespace {

void print_indent(FILE *output, int level)
{
  enum { cPERIOD = 4 };
  putc( '\n', output );
  if (level < 0) {
    putc( '!', output );
    return;
  }
  for( int i = 0; i < level; ++i )
  {
    // Каждые cPERIOD позиций вместо пробела ставим точку.
    bool put_marker = ((i % cPERIOD) == (cPERIOD - 1));

    const char cSpace =  ' ';
    const char cMarker = '.';

    putc( (put_marker ? cMarker : cSpace), output );
  }
}

} // unnamed namespace

void refalrts::vm::print_seq(
  FILE *output, refalrts::Iter begin, refalrts::Iter end
) {
  enum {
    cStateView = 100,
    cStateString,
    cStateFinish
  } state = cStateView;

  int indent = 0;
  bool after_bracket = false;
  bool reset_after_bracket = true;

  while( (state != cStateFinish) && ! refalrts::empty_seq( begin, end ) ) {
    if( reset_after_bracket )
    {
      after_bracket = false;
      reset_after_bracket = false;
    }

    if( after_bracket )
    {
      reset_after_bracket = true;
    }

    switch( state ) {
      case cStateView:
        switch( begin->tag ) {
          case refalrts::cDataIllegal:
            if( 0 == begin->prev ) {
              fprintf( output, "[FIRST] " );
            } else if ( 0 == begin->next ) {
              fprintf( output, "\n[LAST]" );
              state = cStateFinish;
            } else {
              fprintf( output, "\n[NONE]" );
            }
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataSwapHead:
#ifdef MODULE_REFAL
            fprintf( output, "\n\n*Swap %s:\n", (begin->swap_info.name)() );
#else
            fprintf( output, "\n\n*Swap %s:\n", begin->swap_info.name );
#endif
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataChar:
            state = cStateString;
            fprintf( output, "\'" );
            continue;

          case refalrts::cDataNumber:
            fprintf( output, "%ld ", begin->number_info );
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataFunction:
#ifdef MODULE_REFAL
            fprintf( output, "&%s ", (begin->function_info.name)() );
#else
            if( begin->function_info.name[0] != 0 ) {
              fprintf( output, "&%s ", begin->function_info.name );
            } else {
              fprintf( output, "&%p ", begin->function_info.ptr );
            }
#endif
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataIdentifier:
            fprintf( output, "#%s ", (begin->ident_info)() );
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataOpenADT:
            if( ! after_bracket )
            {
              print_indent( output, indent );
            }
            ++indent;
            after_bracket = true;
            reset_after_bracket = false;
            fprintf( output, "[" );
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataCloseADT:
            --indent;
            fprintf( output, "]" );
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataOpenBracket:
            if( ! after_bracket )
            {
              print_indent( output, indent );
            }
            ++indent;
            after_bracket = true;
            reset_after_bracket = false;
            fprintf( output, "(" );
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataCloseBracket:
            --indent;
            fprintf( output, ")" );
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataOpenCall:
            if( ! after_bracket )
            {
              print_indent( output, indent );
            }
            ++indent;
            after_bracket = true;
            reset_after_bracket = false;
            fprintf( output, "<" );
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataCloseCall:
            --indent;
            fprintf( output, ">" );
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataFile:
            fprintf( output, "*%p ", begin->file_info );
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataClosure:
            if( ! after_bracket )
            {
              print_indent( output, indent );
            }
            ++indent;
            after_bracket = true;
            reset_after_bracket = false;
            fprintf( output, "{" );
            begin = unwrap_closure( begin );
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataClosureHead:
            fprintf( output, "[%ld] ", begin->number_info );
            refalrts::move_left( begin, end );
            continue;

          case refalrts::cDataUnwrappedClosure:
            --indent;
            fprintf( output, "}" );
            begin = wrap_closure( begin );
            continue;

          default:
            assert( SWITCH_DEFAULT_VIOLATION );
            throw refalrts::UnexpectedTypeException();
            // break;
        }

      case cStateString:
        switch( begin->tag ) {
          case refalrts::cDataChar: {
            unsigned char ch = static_cast<unsigned char>( begin->char_info );
            switch( ch )
              {
                case '(': case ')':
                case '<': case '>':
                  fprintf( output, "\\%c", ch );
                  break;

                case '\n':
                  fprintf( output, "\\n" );
                  break;

                case '\t':
                  fprintf( output, "\\t" );
                  break;

                case '\\':
                  fprintf( output, "\\\\" );
                  break;

                case '\'':
                  fprintf( output, "\\\'" );
                  break;

                default:
                  if( ch < '\x20' ) {
                    fprintf( output, "\\x%02x", static_cast<int>(ch) );
                  } else {
                    fprintf( output, "%c", ch );
                  }
                  break;
              }
              refalrts::move_left( begin, end );
              continue;
            }

          default:
            state = cStateView;
            fprintf( output, "\' " );
            continue;
        }

      case cStateFinish:
        continue;

      default:
        assert( SWITCH_DEFAULT_VIOLATION );
        throw refalrts::UnexpectedTypeException();
    }
  }

  if( cStateString == state ) {
    fprintf( output, "\'" );
  }
}

void refalrts::vm::make_dump( refalrts::Iter begin, refalrts::Iter end ) {
  using refalrts::vm::dump_stream;

  fprintf( dump_stream(), "\nSTEP NUMBER %u\n", g_step_counter );
  fprintf( dump_stream(), "\nERROR EXPRESSION:\n" );
  print_seq( dump_stream(), begin, end );
  fprintf( dump_stream(), "\nVIEW FIELD:\n" );
  print_seq( dump_stream(), & g_first_marker, & g_last_marker );

#ifdef DUMP_FREE_LIST

  fprintf( dump_stream(), "\nFREE LIST:\n" );
  print_seq(
    dump_stream(),
    & refalrts::allocator::g_first_marker,
    & refalrts::allocator::g_last_marker
  );

#endif //ifdef DUMP_FREE_LIST

  fprintf( dump_stream(),"\nEnd dump\n");
  fflush(dump_stream());
}

FILE *refalrts::vm::dump_stream() {
#ifdef DUMP_FILE

  static FILE *dump_file = 0;

  if( dump_file == 0 ) {
    // Необходимо открыть файл.
    // Если файл не открывается, используем stderr
    dump_file = fopen( DUMP_FILE, "wt" );

    if( dump_file == 0 ) {
      dump_file = stderr;
    }
  }

  return dump_file;

#else //ifdef DUMP_FILE

  return stderr;

#endif //ifdef DUMP_FILE
}

void refalrts::vm::free_view_field() {
  refalrts::Iter begin = g_first_marker.next;
  refalrts::Iter end = & g_last_marker;

  if( begin != end ) {
    end = end->prev;
    refalrts::allocator::splice_to_freelist( begin, end );
  } else {
    /*
      Поле зрения пустое -- его не нужно освобождать.
    */;
  }

#ifndef DONT_PRINT_STATISTICS
  fprintf( stderr, "Step count %d\n", g_step_counter );
#endif // DONT_PRINT_STATISTICS
}

//==============================================================================
// Интерпретатор
//==============================================================================

refalrts::FnResult refalrts::interpret_array(
  const refalrts::RASLCommand raa[],
  refalrts::Iter context[],
  refalrts::Iter begin, refalrts::Iter end,
  const RefalFunction functions[],
  const RefalIdentifier idents[],
  const RefalNumber numbers[],
  const StringItem strings[],
  int open_e_stack[]
) {
  int i = 0;
  Iter res = begin;
  Iter trash_prev = begin->prev;
  unsigned int index;
  int stack_top = 0;

#define MATCH_FAIL \
  { \
    if (stack_top == 0) { \
      return refalrts::cRecognitionImpossible; \
    } else { \
      i = open_e_stack[--stack_top]; \
      continue; \
    } \
  }

  start_sentence();
  while(raa[i].cmd != icEnd)
  {
    // Интерпретация команд
    // Для ряда команд эти переменные могут не иметь смысла
    Iter &bb = context[raa[i].bracket];
    Iter &be = context[raa[i].bracket + 1];
    Iter &elem = context[raa[i].bracket];
    Iter &save_pos = context[raa[i].val1];

    switch(raa[i].cmd)
    {
      case icOnFailGoTo:
        open_e_stack[stack_top++] = i + raa[i].val1 + 1;
        break;

      case icInitB0:
        context[0] = begin;
        context[1] = end;
        refalrts::move_left( context[0], context[1] );
        refalrts::move_left( context[0], context[1] );
        refalrts::move_right( context[0], context[1] );
        break;

      case icInitB0_Lite:
        context[0] = begin;
        context[1] = end;
        break;

      case icCharLeft:
        if ( !char_left( static_cast<char>(raa[i].val2), bb, be) )
          MATCH_FAIL
        break;

      case icCharRight:
        if ( !char_right( static_cast<char>(raa[i].val2), bb, be) )
          MATCH_FAIL
        break;

      case icCharTerm:
        if ( !char_term( static_cast<char>(raa[i].val2), bb ) )
          MATCH_FAIL
        break;

      case icCharLeftSave:
        save_pos = char_left( static_cast<char>(raa[i].val2), bb, be );
        if (! save_pos)
          MATCH_FAIL
        break;

      case icCharRightSave:
        save_pos = char_right( static_cast<char>(raa[i].val2), bb, be );
        if (! save_pos)
          MATCH_FAIL
        break;

      case icNumLeft:
        if( ! refalrts::number_left(
          static_cast<RefalNumber>(raa[i].val2), bb, be )
        )
          MATCH_FAIL
       break;

      case icNumRight:
        if( ! refalrts::number_right(
          static_cast<RefalNumber>(raa[i].val2), bb, be )
        )
          MATCH_FAIL
       break;

      case icNumTerm:
        if(
          ! refalrts::number_term( static_cast<RefalNumber>(raa[i].val2), bb )
        )
          MATCH_FAIL
        break;

      case icNumLeftSave:
        save_pos = number_left( static_cast<RefalNumber>(raa[i].val2), bb, be );
        if (! save_pos)
          MATCH_FAIL
        break;

      case icNumRightSave:
        save_pos =
          number_right( static_cast<RefalNumber>(raa[i].val2), bb, be );
        if (! save_pos)
          MATCH_FAIL
        break;

      case icHugeNumLeft:
        if( ! refalrts::number_left( numbers[raa[i].val2], bb, be ) )
          MATCH_FAIL
        break;

      case icHugeNumRight:
        if( ! refalrts::number_right( numbers[raa[i].val2], bb, be ) )
          MATCH_FAIL
       break;

      case icHugeNumTerm:
        if( ! refalrts::number_term( numbers[raa[i].val2], bb ) )
          MATCH_FAIL
        break;

      case icHugeNumLeftSave:
        save_pos = number_left( numbers[raa[i].val2], bb, be );
        if (! save_pos)
          MATCH_FAIL
        break;

      case icHugeNumRightSave:
        save_pos = number_right( numbers[raa[i].val2], bb, be );
        if (! save_pos)
          MATCH_FAIL
        break;

      case icFuncLeft:
        if ( !function_left( functions[raa[i].val2].ptr, bb, be ) )
          MATCH_FAIL
        break;

      case icFuncRight:
        if ( !function_right( functions[raa[i].val2].ptr, bb, be ) )
          MATCH_FAIL
        break;

      case icFuncTerm:
        if ( !function_term( functions[raa[i].val2].ptr, bb ) )
          MATCH_FAIL
        break;

      case icFuncLeftSave:
        save_pos = function_left( functions[raa[i].val2].ptr, bb, be );
        if (! save_pos)
          MATCH_FAIL
        break;

      case icFuncRightSave:
        save_pos = function_right( functions[raa[i].val2].ptr, bb, be );
        if (! save_pos)
          MATCH_FAIL
        break;

      case icIdentLeft:
        if( ! refalrts::ident_left( idents[raa[i].val2], bb, be ) )
          MATCH_FAIL
        break;

      case icIdentRight:
        if( ! refalrts::ident_right( idents[raa[i].val2], bb, be ) )
          MATCH_FAIL
        break;

      case icIdentTerm:
        if( ! refalrts::ident_term( idents[raa[i].val2], bb ) )
          MATCH_FAIL
        break;

      case icIdentLeftSave:
        save_pos = ident_left( idents[raa[i].val2], bb, be );
        if (! save_pos)
          MATCH_FAIL
        break;

      case icIdentRightSave:
        save_pos = ident_right( idents[raa[i].val2], bb, be );
        if (! save_pos)
          MATCH_FAIL
        break;

      case icBracketLeft:
        if( !refalrts::brackets_left( context[raa[i].val2],
                                      context[raa[i].val2 + 1],
                                      bb, be )
        )
          MATCH_FAIL
        break;

      case icBracketRight:
        if( !refalrts::brackets_right( context[raa[i].val2],
                                       context[raa[i].val2 + 1],
                                       bb, be )
        )
          MATCH_FAIL
        break;

      case icBracketTerm:
        if( !refalrts::brackets_term( context[raa[i].val2],
                                      context[raa[i].val2 + 1],
                                      bb )
        )
          MATCH_FAIL
        break;

      case icBracketLeftSave:
        {
          int inner = raa[i].val2;
          context[inner + 2] =
            brackets_left(context[inner], context[inner + 1], bb, be);
          if (! context[inner + 2])
            MATCH_FAIL
          bracket_pointers(context[inner + 2], context[inner + 3]);
        }
        break;

      case icBracketRightSave:
        {
          int inner = raa[i].val2;
          context[inner + 2] =
            brackets_right(context[inner], context[inner + 1], bb, be);
          if (! context[inner + 2])
            MATCH_FAIL
          bracket_pointers(context[inner + 2], context[inner + 3]);
        }
        break;

      case icADTLeft:
        if( ! refalrts::adt_left( context[raa[i].val2],
                                  context[raa[i].val2 + 1],
                                  functions[raa[i].val1].ptr,
                                  bb, be)
        )
          MATCH_FAIL
        break;

      case icADTRight:
        if( ! refalrts::adt_right( context[raa[i].val2],
                                   context[raa[i].val2 + 1],
                                   functions[raa[i].val1].ptr,
                                   bb, be)
        )
          MATCH_FAIL
        break;

      case icADTTerm:
        if( ! refalrts::adt_term( context[raa[i].val2],
                                  context[raa[i].val2 + 1],
                                  functions[raa[i].val1].ptr,
                                  bb )
        )
          MATCH_FAIL
        break;

      case icADTLeftSave:
        {
          int inner = raa[i].val2;
          RefalFunctionPtr tag = functions[raa[i].val1].ptr;
          context[inner + 2] =
            adt_left(context[inner], context[inner + 1], tag, bb, be);
          if (! context[inner + 2])
            MATCH_FAIL
          adt_pointers(context[inner + 2], context[inner + 3], context[inner + 4]);
        }
        break;

      case icADTRightSave:
        {
          int inner = raa[i].val2;
          RefalFunctionPtr tag = functions[raa[i].val1].ptr;
          context[inner + 2] =
            adt_right(context[inner], context[inner + 1], tag, bb, be);
          if (! context[inner + 2])
            MATCH_FAIL
          adt_pointers(context[inner + 2], context[inner + 3], context[inner + 4]);
        }
        break;

      case icCallSaveLeft:
        context[raa[i].val2 + 2] =
          refalrts::call_left(
            context[raa[i].val2], context[raa[i].val2 + 1], bb, be
          );
        break;

      case icEmpty:
        if ( !empty_seq( bb, be ) )
          MATCH_FAIL
        break;

      case icsVarLeft:
        index = raa[i].val2;
        if( !refalrts::svar_left( context[index], bb, be) )
          MATCH_FAIL
        break;

      case icsVarRight:
        index = raa[i].val2;
        if( !refalrts::svar_right( context[index], bb, be) )
          MATCH_FAIL
        break;

      case icsVarTerm:
        if( !refalrts::svar_term( bb, bb ) )
          MATCH_FAIL
        break;

      case ictVarLeft:
        index = raa[i].val2;
        if( !refalrts::tvar_left( context[index], bb, be) )
          MATCH_FAIL
        break;

      case ictVarRight:
        index = raa[i].val2;
        if( !refalrts::tvar_right( context[index], bb, be) )
          MATCH_FAIL
       break;

      case ictVarLeftSave:
        index = raa[i].val2;
        context[index + 1] = tvar_left(context[index], bb, be);
        if (! context[index + 1])
          MATCH_FAIL
        break;

      case ictVarRightSave:
        index = raa[i].val2;
        context[index + 1] = tvar_right(context[index], bb, be);
        if (! context[index + 1])
          MATCH_FAIL
        break;

      case iceRepeatLeft:
        {
          int index = raa[i].val1;
          int sample = raa[i].val2;
          if( ! refalrts::repeated_evar_left( context[index], context[index + 1],
                                              context[sample], context[sample + 1],
                                              bb, be)
          )
            MATCH_FAIL
        }
        break;

      case iceRepeatRight:
        {
          int index = raa[i].val1;
          int sample = raa[i].val2;
          if( ! refalrts::repeated_evar_right( context[index], context[index + 1],
                                               context[sample], context[sample + 1],
                                               bb, be)
          )
            MATCH_FAIL
        }
        break;

      case icsRepeatLeft:
      case ictRepeatLeft:
        {
          int index = raa[i].val1;
          int sample = raa[i].val2;
          if( ! refalrts::repeated_stvar_left( context[index],  context[sample],
                                               bb, be) )
            MATCH_FAIL
        }
        break;

      case icsRepeatRight:
      case ictRepeatRight:
        {
          int index = raa[i].val1;
          int sample = raa[i].val2;
          if( ! refalrts::repeated_stvar_right( context[index],  context[sample],
                                                bb, be) )
            MATCH_FAIL
        }
        break;

      case icsRepeatTerm:
      case ictRepeatTerm:
        assert(raa[i].bracket == raa[i].val1);
        if( ! refalrts::repeated_stvar_term( context[raa[i].val2], bb ) )
          MATCH_FAIL
        break;

      case ictRepeatLeftSave:
        {
          int index = raa[i].val1;
          int sample = raa[i].val2;

          context[index + 1] =
            repeated_stvar_left(context[index], context[sample], bb, be);
          if (! context[index + 1])
            MATCH_FAIL
        }
        break;

      case ictRepeatRightSave:
        {
          int index = raa[i].val1;
          int sample = raa[i].val2;

          context[index + 1] =
            repeated_stvar_right(context[index], context[sample], bb, be);
          if (! context[index + 1])
            MATCH_FAIL
        }
        break;

      case icEPrepare:
        context[raa[i].val2] = 0;
        context[raa[i].val2 + 1] = 0;
        open_e_stack[stack_top++] = ++i;
        start_e_loop();
        break;

      case icEStart:
        {
          bool advance = open_evar_advance(
            context[raa[i].val2], context[raa[i].val2 + 1], bb, be
          );
          if ( ! advance ) {
            MATCH_FAIL
          }
          open_e_stack[stack_top++] = i;
        }
        break;

      case icSave:
        context[raa[i].val2] = bb;
        context[raa[i].val2 + 1] = be;
        break;

      case icEmptyResult:
        reset_allocator();
        break;

      case icSetResRightEdge:
        res = end->next;
        break;

      case icSetRes:
        res = context[raa[i].bracket];
        break;

      case icCopyEVar:
        {
          unsigned int target = raa[i].val1;
          unsigned int sample = raa[i].val2;
          if (
            ! copy_evar(
              context[target], context[target + 1],
              context[sample], context[sample + 1]
            )
          )
            return cNoMemory;
        }
        break;

      case icCopySTVar:
        {
          unsigned int target = raa[i].val1;
          unsigned int sample = raa[i].val2;
          if (! copy_stvar(context[target], context[sample]))
            return cNoMemory;
        }
        break;

      case icReinitSVar:
        reinit_svar(elem, context[raa[i].val2]);
        break;

      case icAllocChar:
        if(!alloc_char(elem, static_cast<char>(raa[i].val2)))
          return cNoMemory;
        break;

      case icAllocFunc:
        if(
            !alloc_name(
              elem,
              functions[raa[i].val2].ptr,
              functions[raa[i].val2].name
            )
        )
          return cNoMemory;
        break;

      case icAllocInt:
        if(!alloc_number(elem, static_cast<RefalNumber>(raa[i].val2)))
          return cNoMemory;
        break;

      case icAllocHugeInt:
        if(!alloc_number(elem, numbers[raa[i].val2]))
          return cNoMemory;
        break;

      case icAllocIdent:
        if(!alloc_ident(elem, idents[raa[i].val2]))
          return cNoMemory;
        break;

      case icAllocBracket:
        switch(raa[i].val2)
        {
          case ibOpenADT:
            if(!alloc_open_adt(elem))
              return cNoMemory;
            break;

          case ibOpenBracket:
            if(!alloc_open_bracket(elem))
              return cNoMemory;
            break;

          case ibOpenCall:
            if(!alloc_open_call(elem))
              return cNoMemory;
            break;

          case ibCloseADT:
            if(!alloc_close_adt(elem))
              return cNoMemory;
            break;

          case ibCloseBracket:
            if(!alloc_close_bracket(elem))
              return cNoMemory;
            break;

          case ibCloseCall:
            if(!alloc_close_call(elem))
              return cNoMemory;
            break;

          default:
            throw UnexpectedTypeException();
        }
        break;

      case icAllocString:
        {
          if (
            ! alloc_chars(
              bb, be,
              strings[raa[i].val2].string, strings[raa[i].val2].string_len
            )
          )
            return cNoMemory;
        }
        break;

      case icReinitChar:
        reinit_char(elem, static_cast<char>(raa[i].val2));
        break;

      case icReinitFunc:
        reinit_name(
          elem, functions[raa[i].val2].ptr, functions[raa[i].val2].name
        );
        break;

      case icReinitInt:
        reinit_number(elem, static_cast<RefalNumber>(raa[i].val2));
        break;

      case icReinitHugeInt:
        reinit_number(elem, numbers[raa[i].val2]);
        break;

      case icReinitIdent:
        reinit_ident(elem, idents[raa[i].val2]);
        break;

      case icReinitBracket:
        switch (raa[i].val2) {
          case ibOpenADT:
            reinit_open_adt(elem);
            break;

          case ibOpenBracket:
            reinit_open_bracket(elem);
            break;

          case ibOpenCall:
            reinit_open_call(elem);
            break;

          case ibCloseADT:
            reinit_close_adt(elem);
            break;

          case ibCloseBracket:
            reinit_close_bracket(elem);
            break;

          case ibCloseCall:
            reinit_close_call(elem);
            break;

          default:
            throw UnexpectedTypeException();
        }
        break;

      case icUpdateChar:
        update_char(elem, static_cast<char>(raa[i].val2));
        break;

      case icUpdateFunc:
        update_name(
          elem, functions[raa[i].val2].ptr, functions[raa[i].val2].name
        );
        break;

      case icUpdateInt:
        update_number(elem, static_cast<RefalNumber>(raa[i].val2));
        break;

      case icUpdateHugeInt:
        update_number(elem, numbers[raa[i].val2]);
        break;

      case icUpdateIdent:
        update_ident(elem, idents[raa[i].val2]);
        break;

      case icLinkBrackets:
        link_brackets(context[raa[i].val1], context[raa[i].val2]);
        break;

      case icPushStack:
        push_stack(elem);
        break;

      case icSpliceElem:
        res = splice_elem(res, elem);
        break;

      case icSpliceEVar:
        res = splice_evar(res, bb, be);
        break;

      case icSpliceSTVar:
        res = splice_stvar(res, elem);
        break;

      case icSpliceRange:
        res = splice_evar(res, bb, be);
        break;

      case icSpliceTile:
        res = splice_evar(res, context[raa[i].val1], context[raa[i].val2]);
        break;

      case icReturnResult:
        splice_to_freelist(begin, end);
        return cSuccess;

      case icReturnResult_NoTrash:
        return cSuccess;

      case icTrashLeftEdge:
        splice_to_freelist_open(trash_prev, res);
        break;

      case icTrash:
        splice_to_freelist_open(context[raa[i].bracket], res);
        break;

      case icFail:
        MATCH_FAIL;

      default:
        printf( "bad command: %d\n", raa[i].cmd );
        assert( SWITCH_DEFAULT_VIOLATION );
        throw UnexpectedTypeException();
    }
    i++;
  }

  return cSuccess;
}


const refalrts::RefalFunction refalrts::functions[] = { { 0, 0 } };
const refalrts::RefalIdentifier refalrts::idents[] = { 0 };
const refalrts::RefalNumber refalrts::numbers[] = { 0 };
const refalrts::StringItem refalrts::strings[] = { { "", 0 } };


//==============================================================================

// Используются в Library.cpp

char **g_argv = 0;
int g_argc = 0;

int main(int argc, char **argv) {
  g_argc = argc;
  g_argv = argv;

  refalrts::FnResult res;
  try {
    refalrts::vm::init_view_field();
    refalrts::profiler::start_profiler();
    res = refalrts::vm::main_loop();
    fflush(stderr);
    fflush(stdout);
  } catch (refalrts::UnexpectedTypeException) {
    fprintf(stderr, "INTERNAL ERROR: check all switches\n");
    return 3;
  } catch (std::exception& e) {
    fprintf(stderr, "INTERNAL ERROR: std::exception %s\n", e.what());
    return 4;
  } catch (...) {
    fprintf(stderr, "INTERNAL ERROR: unknown exception\n");
    return 5;
  }

  refalrts::profiler::end_profiler();
  refalrts::vm::free_view_field();
  refalrts::allocator::free_memory();

  fflush(stdout);

  switch( res ) {
    case refalrts::cSuccess:
      return 0;

    case refalrts::cRecognitionImpossible:
      return 101;

    case refalrts::cNoMemory:
      return 102;

    case refalrts::cExit:
      return refalrts::vm::g_ret_code;

    default:
      fprintf(stderr, "INTERNAL ERROR: check switch in main (res = %d)\n", res);
      return 105;
  }
}
