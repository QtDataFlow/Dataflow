
//  Copyright (c) 2014 - 2020 Maksym V. Bilinets.
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

#include <dataflow/prelude/stateful/internal/transition.h>

namespace dataflow
{
namespace stateful
{
namespace internal
{

transition::transition()
: idx_()
, t_()
{
}

transition::transition(integer idx, const dtimestamp& when)
: idx_(idx)
, t_(when)
{
}

bool transition::operator==(const transition& other) const
{
  return idx_ == other.idx_ && t_ == other.t_;
}

bool transition::operator!=(const transition& other) const
{
  return !(*this == other);
}

integer transition::index() const
{
  return idx_;
}

const dtimestamp& transition::when() const
{
  return t_;
}

} // internal
} // stateful

std::ostream& stateful::internal::operator<<(std::ostream& out,
                                             const transition& value)
{
  return out << "transition(" << value.index() << "; " << value.when() << ")";
}

ref<stateful::internal::transition>
stateful::internal::Transition(const arg<integer>& idx, dtime t)
{
  struct policy
  {
    static std::string label()
    {
      return "transition";
    }
    static transition calculate(const integer& idx, const dtimestamp& when)
    {
      return {idx, when};
    }
  };

  return core::Lift(policy(), idx, Timestamp(t));
}

ref<integer> stateful::internal::Index(const ref<transition>& tr)
{
  struct policy
  {
    static std::string label()
    {
      return "transition-index";
    }
    static integer calculate(const transition& tr)
    {
      return tr.index();
    }
  };

  return core::Lift(policy(), tr);
}

ref<dtimestamp> stateful::internal::When(const ref<transition>& tr)
{
  struct policy
  {
    static std::string label()
    {
      return "transition-when";
    }
    static dtimestamp calculate(const transition& tr)
    {
      return tr.when();
    }
  };

  return core::Lift(policy(), tr);
}

} // dataflow
