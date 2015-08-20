/*
Copyright (c) 2015 Geoffrey Jones 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */

#pragma once
#include <iomanip>
#include <type_traits>
#include <cstring>

/* This is a  little helper class for the times when you need to print variables
 * to a stream output (cout,stringstream,etc) there is little or no error
 * checking so if your stream does not support the iomanip functions used, this
 * will break... but at compile time: yay!
 * two version exist:
 *     print(streamout, args...)
 *     fprint(streamout,fmt,args)
 * for the formating version fprint(...) the emphasis was to keep things minimal
 * so the varibles get substituted into '{...}' sections or if the sections run
 * out they get tacked on the end. Likewise if variables run out the remainder
 * of the format string is flushed braces and all.
 */
namespace streamprint {
namespace detail {

template <typename T, typename... Args>
constexpr auto first(T &&t, Args &&... args) {
  return t;
}

template <typename T> constexpr auto first(T &&t) { return t; }

template <typename T, typename... Args>
auto print_h(T &out, Args &&... args) -> std::enable_if_t<1 < sizeof...(Args)> {
  out << first(args...);
  auto l = [&out](auto &o, auto &&, auto &&... a) { print_h(o, a...); };
  l(out, args...);
}

template <typename T, typename... Args>
auto print_h(T &out, Args &&... args)
    -> std::enable_if_t<1 == sizeof...(Args)> {
  out << first(args...);
}

template <typename T, typename... Args>
auto print_h(T &out, Args &&... args)
    -> std::enable_if_t<0 == sizeof...(Args)> {}

template <typename T> void setOpts(T &out, char *optstr) {
  auto setopt = [optstr](char *opt, auto f) {
    if (opt > optstr && *(opt - 1) == 'f') {
      // the found value was actually for a fill char
      if ((opt = strchr(opt + 1, *opt)) != nullptr)
        f(opt + 1);
    } else
      f(opt + 1);
  };
  char *opt = nullptr;
  if ((opt = strchr(optstr, 'w')) != nullptr) {
    setopt(opt, [&out](char *o) { out << std::setw(atoi(o)); });
  }
  if ((opt = strchr(optstr, 'p')) != nullptr) {
    setopt(opt, [&out](char *o) { out << std::setprecision(atoi(o)); });
  }
  if ((opt = strchr(optstr, 'm')) != nullptr) {
    setopt(opt, [&out](char *o) {
      switch (*o) {
      case 'x':
        out << std::fixed;
        break;
      case 's':
        out << std::scientific;
        break;
      case 'h':
        out << std::hexfloat;
        break;
      case 'd':
        out << std::defaultfloat;
        break;
      }
    });
  }
  if ((opt = strchr(optstr, 'f')) != nullptr) {
    out << std::setfill(*(opt + 1));
  }
  if ((opt = strchr(optstr, 'L')) != nullptr) {
    out << std::left;
  }
  if ((opt = strchr(optstr, 'R')) != nullptr) {
    out << std::right;
  }
}
template <typename T> char *checkSetOpts(T &out, char *f) {
  if (f != nullptr) {
    auto b1 = strchr(f, '{');
    auto b2 = strchr(f, '}');
    if (b1 != nullptr)
      (*b1) = char(0);
    if (b2 != nullptr)
      (*b2) = char(0);
    out << f;
    if (b1 != nullptr)
      setOpts(out, b1 + 1);
    f = b2 == nullptr ? b2 : b2 + 1;
    if (b1 != nullptr)
      (*b1) = char(1);
    if (b2 != nullptr)
      (*b2) = char(1);
  }
  return f;
}

template <typename T, typename... Args>
auto fprint_h(T &out, char *f, Args &&... args)
    -> std::enable_if_t<1 < sizeof...(Args)> {
  f = checkSetOpts(out, f);
  out << first(args...);
  auto l = [](auto &o, char *f_, auto &&, auto &&... a) {
    fprint_h(o, f_, a...);
  };
  l(out, f, args...);
}

template <typename T, typename... Args>
auto fprint_h(T &out, char *f, Args &&... args)
    -> std::enable_if_t<1 == sizeof...(Args)> {
  f = checkSetOpts(out, f);
  out << first(args...);
  fprint_h(out, f);
}

template <typename T, typename... Args>
auto fprint_h(T &out, char *f, Args &&... args)
    -> std::enable_if_t<0 == sizeof...(Args)> {
  checkSetOpts(out, f);
}
}
/* Just wham all the args at the out using stream out opperator (<<) no
 * formatting will be applied and there is not delim placed between outputed
 * variables
 * @out the output stream
 * @args the variables
 */
template <typename T, typename... Args>
auto print(T &out, Args &&... args) -> T & {
  detail::print_h(out, args...);
  return out;
}
/* Format print the arguments to the output stream for specialisation for format
 * string known at compile time. Variables will be streamed to the output
 * according to the format string
 * @out the output stream
 * @f the format string:
 *     variables are substituted into '{..}' locations according to the format
 *     specified within that set of braces. Formatting options are:
 *        f[char] - set fill character
 *        w[num]  - set stream width
 *        p[num]  - set stream precsision
 *        m[xshd] - set float number format x:fixed, s:scientific, h:hexfloat,
 *                  d:default
 *        RL      - set align left or righ
 *     for example {f w10mxp3} means ' ' filled 10 wide fixed point 3 precsision
 * @args the variables
 * e.g. fprint(outstream,"arg0 in -> {mdp5} arg1 in -> {f w15}...",arg0,arg1,...)
 */
template <typename T, size_t N, typename... Args>
auto fprint(T &out, const char(&f)[N], Args &&... v)
    -> std::enable_if_t<(N > 1), T &> {
  char fcpy[N];
  memcpy(fcpy, f, N);
  detail::fprint_h(out, fcpy, v...);
  return out;
}
/* Format print the arguments to the output stream for specialisation for format
 * string known to be empty at compile time. Variables will be streamed to the
 * output according to the format string
 * @out the output stream
 * @f the format string:
 *     variables are substituted into '{..}' locations according to the format
 *     specified within that set of braces. Formatting options are:
 *        f[char] - set fill character
 *        w[num]  - set stream width
 *        p[num]  - set stream precsision
 *        m[xshd] - set float number format x:fixed, s:scientific, h:hexfloat,
 *                  d:default
 *        RL      - set align left or righ
 *     for example {f w10mxp3} means ' ' filled 10 wide fixed point 3 precsision
 * @args the variables
 * e.g. fprint(outstream,"",arg0,arg1,...)
 */
template <typename T, size_t N, typename... Args>
auto fprint(T &out, const char(&f)[N], Args &&... v)
    -> std::enable_if_t<N == 1, T &> {
  detail::print_h(out, v...);
  return out;
}
/* Format print the arguments to the output stream for specialisation for format
 * string run time created fromat strings (eg string_obj.c_str()). Variables will be streamed to the
 * output according to the format string
 * @out the output stream
 * @f the format string:
 *     variables are substituted into '{..}' locations according to the format
 *     specified within that set of braces. Formatting options are:
 *        f[char] - set fill character
 *        w[num]  - set stream width
 *        p[num]  - set stream precsision
 *        m[xshd] - set float number format x:fixed, s:scientific, h:hexfloat,
 *                  d:default
 *        RL      - set align left or righ
 *     for example {f w10mxp3} means ' ' filled 10 wide fixed point 3 precsision
 * @args the variables
 * e.g. fprint(outstream,fmt_ptr,arg0,arg1,...)
 */
template <typename T, typename C, typename... Args>
auto fprint(T &out, C f, Args &&... v) -> std::enable_if_t<
    std::is_pointer<C>::value &&
        std::is_same<std::decay_t<std::remove_pointer_t<C>>, char>::value,
    T &> {
  const size_t slen = strlen(f) + 1;
  if (slen) {
    char *fcpy = new char[slen];
    memcpy(fcpy, f, slen);
    detail::fprint_h(out, fcpy, v...);
    delete[] fcpy;
    return out;
  }
  return print(out, v...);
}
}
