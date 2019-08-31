
//  Copyright (c) 2014 - 2019 Maksym V. Bilinets.
//
//  This file is part of Dataflow++.
//
//  Dataflow++ is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Dataflow++ is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with Dataflow++. If not, see <http://www.gnu.org/licenses/>.

#include <dataflow/prelude/stateful.h>

#include <dataflow/introspect.h>

#include <boost/mpl/list.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/test/unit_test.hpp>

using namespace dataflow;

namespace dataflow_test
{

BOOST_AUTO_TEST_SUITE(test_stateful)

using good_transitions_function_types = boost::mpl::list<
  std::function<std::tuple<std::pair<ref<bool>, int>>(ref<int>)>,
  std::function<std::tuple<std::pair<ref<bool>, ref<int>>>(ref<int>)>,
  std::function<std::tuple<std::pair<ref<bool>, val<int>>>(ref<int>)>,
  std::function<std::tuple<std::pair<ref<bool>, var<int>>>(ref<int>)>,
  std::function<std::tuple<
    std::pair<ref<bool>, std::function<ref<int>(const Time&)>>>(ref<int>)>,
  std::function<std::tuple<
    std::pair<ref<bool>, std::function<val<int>(const Time&)>>>(ref<int>)>,
  std::function<std::tuple<
    std::pair<ref<bool>, std::function<var<int>(const Time&)>>>(ref<int>)>,
  std::function<std::tuple<
    std::pair<ref<bool>, int>,
    std::pair<ref<bool>, ref<int>>,
    std::pair<ref<bool>, val<int>>,
    std::pair<ref<bool>, var<int>>,
    std::pair<ref<bool>, std::function<ref<int>(const Time&)>>,
    std::pair<ref<bool>, std::function<val<int>(const Time&)>>,
    std::pair<ref<bool>, std::function<var<int>(const Time&)>>>(ref<int>)>,
  std::tuple<std::pair<ref<bool>, int>> (*)(ref<int>),
  std::tuple<std::pair<ref<bool>, ref<int>>> (*)(ref<int>),
  std::tuple<std::pair<ref<bool>, val<int>>> (*)(ref<int>),
  std::tuple<std::pair<ref<bool>, var<int>>> (*)(ref<int>),
  std::tuple<std::pair<ref<bool>, int>,
             std::pair<ref<bool>, ref<int>>,
             std::pair<ref<bool>, val<int>>,
             std::pair<ref<bool>, var<int>>,
             std::pair<ref<bool>, ref<int> (*)(const Time&)>,
             std::pair<ref<bool>, val<int> (*)(const Time&)>,
             std::pair<ref<bool>, var<int> (*)(const Time&)>> (*)(ref<int>)>;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_is_transitions_function,
                              T,
                              good_transitions_function_types)
{
  BOOST_CHECK_EQUAL((stateful::is_transitions_function<T, int>::value), true);

  BOOST_CHECK_EQUAL((stateful::is_transitions_function<T, char>::value), false);
  BOOST_CHECK_EQUAL((stateful::is_transitions_function<T, float>::value),
                    false);
  BOOST_CHECK_EQUAL((stateful::is_transitions_function<T, void>::value), false);
  BOOST_CHECK_EQUAL((stateful::is_transitions_function<T, const char*>::value),
                    false);
}

using bad_transitions_function_types = boost::mpl::list<
  std::function<std::tuple<std::pair<ref<bool>, char>>(ref<int>)>,
  std::function<std::tuple<std::pair<ref<bool>, ref<char>>>(ref<int>)>,
  std::function<std::tuple<std::pair<ref<bool>, ref<int>>>(ref<char>)>,
  std::function<std::tuple<std::pair<ref<char>, ref<int>>>(ref<int>)>,
  std::function<std::tuple<
    std::pair<ref<bool>, std::function<ref<char>(const Time&)>>>(ref<int>)>,
  std::function<std::tuple<
    std::pair<ref<bool>, std::function<ref<int>(const Time&)>>>(ref<char>)>,
  std::function<std::tuple<
    std::pair<ref<char>, std::function<ref<int>(const Time&)>>>(ref<int>)>,
  std::function<std::tuple<std::pair<ref<bool>, ref<int>>,
                           std::pair<ref<bool>, char>>(ref<int>)>,
  std::tuple<std::pair<ref<bool>, char>> (*)(ref<int>),
  std::tuple<std::pair<ref<bool>, ref<char>>> (*)(ref<int>),
  std::tuple<std::pair<ref<bool>, ref<int>>> (*)(ref<char>),
  std::tuple<std::pair<ref<char>, var<int>>> (*)(ref<int>),
  std::tuple<std::pair<ref<bool>, ref<int>>,
             std::pair<ref<bool>, ref<char>>> (*)(ref<int>),
  int,
  void>;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_is_transitions_function_false,
                              T,
                              bad_transitions_function_types)
{
  BOOST_CHECK_EQUAL((stateful::is_transitions_function<T, int>::value), false);
  BOOST_CHECK_EQUAL((stateful::is_transitions_function<T, char>::value), false);
  BOOST_CHECK_EQUAL((stateful::is_transitions_function<T, float>::value),
                    false);
  BOOST_CHECK_EQUAL((stateful::is_transitions_function<T, void>::value), false);
  BOOST_CHECK_EQUAL((stateful::is_transitions_function<T, const char*>::value),
                    false);
}

BOOST_AUTO_TEST_SUITE_END()

} // dataflow_test
