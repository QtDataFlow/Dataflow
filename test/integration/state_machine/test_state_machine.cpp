
//  Copyright (c) 2014 - 2017 Maksym V. Bilinets.
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

#include "../../tools/graph_invariant.h"

#include <dataflow/introspect.h>
#include <dataflow/prelude.h>
#include <dataflow/tuple.h>

#include <boost/test/unit_test.hpp>

using namespace dataflow;

namespace dataflow_test
{

namespace
{
enum class mode
{
  hovering,
  dragging
};

std::ostream& operator<<(std::ostream& out, mode value)
{
  out << "mode::" << (value == mode::hovering ? "hovering" : "dragging");
  return out;
}

class point
{
public:
  explicit point(int x = 0, int y = 0)
  : x_(x)
  , y_(y)
  {
  }

  bool operator==(const point& other) const
  {
    return x_ == other.x_ && y_ == other.y_;
  }

  bool operator!=(const point& other) const
  {
    return !(*this == other);
  }

  point operator-(const point& other) const
  {
    return point(x_ - other.x_, y_ - other.y_);
  }

  point operator+(const point& other) const
  {
    return point(x_ + other.x_, y_ + other.y_);
  }

  friend std::ostream& operator<<(std::ostream& out, const point& value)
  {
    out << "point (" << value.x_ << "; " << value.y_ << ")";
    return out;
  }

  friend ref<int> Distance(const ref<point>& a, const ref<point>& b)
  {
    struct policy
    {
      static std::string label()
      {
        return "distance";
      }
      int calculate(const point& a, const point& b)
      {
        const point v = a - b;
        return std::sqrt(v.x_ * v.x_ + v.y_ * v.y_);
      }
    };
    return core::Lift<policy>(a, b);
  }

private:
  int x_;
  int y_;
};
}

ref<point> AdjustableCirclePosition(const ref<point>& initial_circle_pos,
                                    const ref<int>& radius,
                                    const ref<point>& mouse_pos,
                                    const ref<bool>& mouse_pressed,
                                    const Time& t0)
{
  const auto tf = [=](const ref<tupleE<mode, point, point, point>>& sp)
  {
    auto prev_mode = First(sp);
    auto circle_pos = Second(sp);
    auto starting_circle_pos = Third(sp);
    auto starting_mouse_pos = Fourth(sp);

    return If(prev_mode == mode::hovering,
              If(mouse_pressed && Distance(mouse_pos, circle_pos) < radius,
                 TupleE(mode::dragging, circle_pos, circle_pos, mouse_pos),
                 sp),
              If(!mouse_pressed,
                 TupleE(mode::hovering, circle_pos, point(), point()),
                 TupleE(mode::dragging,
                        starting_circle_pos + (mouse_pos - starting_mouse_pos),
                        starting_circle_pos,
                        starting_mouse_pos)));
  };

  const auto s = StateMachine(
    TupleE(mode::hovering, initial_circle_pos, point(), point()), tf, t0);

  return Second(s);
}

BOOST_AUTO_TEST_SUITE(test_integration_state_machine)

BOOST_AUTO_TEST_CASE(test_drag_and_drop)
{
  Engine engine;

  auto mouse_pressed = Var<bool>(false);
  auto mouse_pos = Var(point(0, 0));

  auto f =
    Main([=](const Time& t0)
         {
           return AdjustableCirclePosition(
             Const(point(100, 100)), Const(30), mouse_pos, mouse_pressed, t0);
         });

  BOOST_CHECK(graph_invariant_holds());

  BOOST_CHECK_EQUAL(f(), (point(100, 100)));

  mouse_pos = point(90, 110);

  BOOST_CHECK(graph_invariant_holds());

  mouse_pressed = true;

  BOOST_CHECK(graph_invariant_holds());

  mouse_pos = point(200, 100);

  BOOST_CHECK(graph_invariant_holds());

  BOOST_CHECK_EQUAL(f(), point(210, 90));

  mouse_pos = point(150, 150);

  BOOST_CHECK(graph_invariant_holds());

  BOOST_CHECK_EQUAL(f(), point(160, 140));

  mouse_pressed = false;

  BOOST_CHECK(graph_invariant_holds());

  mouse_pos = point(200, 100);

  BOOST_CHECK(graph_invariant_holds());

  BOOST_CHECK_EQUAL(f(), point(160, 140));

  mouse_pos = point(90, 110);

  BOOST_CHECK(graph_invariant_holds());

  mouse_pressed = true;

  BOOST_CHECK(graph_invariant_holds());

  mouse_pos = point(200, 100);

  BOOST_CHECK(graph_invariant_holds());

  BOOST_CHECK_EQUAL(f(), point(160, 140));

  mouse_pressed = false;

  BOOST_CHECK(graph_invariant_holds());

  mouse_pos = point(150, 150);

  BOOST_CHECK(graph_invariant_holds());

  mouse_pressed = true;

  BOOST_CHECK(graph_invariant_holds());

  mouse_pos = point(170, 170);

  BOOST_CHECK(graph_invariant_holds());

  BOOST_CHECK_EQUAL(f(), point(180, 160));
}

BOOST_AUTO_TEST_SUITE_END()
}