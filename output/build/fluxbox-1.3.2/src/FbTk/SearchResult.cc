// SearchResult.cc for FbTk - Fluxbox Toolkit
// Copyright (c) 2007 Fluxbox Team (fluxgen at fluxbox dot org)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "SearchResult.hh"
#include "ITypeAheadable.hh"

namespace FbTk {

void SearchResult::seek() {
    switch (m_results.size()) {
    case 0:
        break;
    case 1:
        m_seeked_string = m_results[0]->iTypeString();
        break;
    default:
        bool seekforward = true;
        for (size_t i=1; i < m_results.size() && seekforward &&
            m_results[0]->iTypeCheckStringSize(m_seeked_string.size()); i++) {
            if (!m_results[i]->iTypeCompareChar(
                    m_results[0]->iTypeChar(m_seeked_string.size()),
                    m_seeked_string.size())) {
                seekforward = false;
            } else if (i == m_results.size() - 1) {
                m_seeked_string += m_results[0]->iTypeChar(m_seeked_string.size());
                i = 0;
            }
        }
        break;
    }
}

} // end namespace FbTk
