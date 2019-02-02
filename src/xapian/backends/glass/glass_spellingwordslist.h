/** @file glass_spellingwordslist.h
 * @brief A termlist containing all words which are spelling targets.
 */
/* Copyright (C) 2005,2008,2009,2010,2011,2017 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_GLASS_SPELLINGWORDSLIST_H
#define XAPIAN_INCLUDED_GLASS_SPELLINGWORDSLIST_H

#include "backends/alltermslist.h"
#include "glass_spelling.h"
#include "glass_cursor.h"

class GlassDatabase;

class GlassSpellingWordsList : public AllTermsList {
    /// Copying is not allowed.
    GlassSpellingWordsList(const GlassSpellingWordsList &);

    /// Assignment is not allowed.
    void operator=(const GlassSpellingWordsList &);

    /// Keep a reference to our database to stop it being deleted.
    Xapian::Internal::intrusive_ptr<const GlassDatabase> database;

    /** A cursor which runs through the spelling table reading termnames from
     *  the keys.
     */
    GlassCursor * cursor;

    /** The term frequency of the term at the current position.
     *
     *  If this value is zero, then we haven't read the term frequency or
     *  collection frequency for the current term yet.  We need to call
     *  read_termfreq() to read these.
     */
    mutable Xapian::termcount termfreq;

    /// Read and cache the term frequency.
    void read_termfreq() const;

  public:
    GlassSpellingWordsList(Xapian::Internal::intrusive_ptr<const GlassDatabase> database_,
			   GlassCursor * cursor_)
	    : database(database_), cursor(cursor_), termfreq(0) {
	// Seek to the entry before the first key with a "W" prefix, so the
	// first next() will advance us to the first such entry.
	cursor->find_entry(std::string("W", 1));
    }

    /// Destructor.
    ~GlassSpellingWordsList();

    Xapian::termcount get_approx_size() const;

    /** Returns the current termname.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    std::string get_termname() const;

    /** Returns the term frequency of the current term.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    Xapian::doccount get_termfreq() const;

    /** Returns the collection frequency of the current term.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    Xapian::termcount get_collection_freq() const;

    /// Advance to the next term in the list.
    TermList * next();

    /// Advance to the first term which is >= tname.
    TermList * skip_to(const std::string &tname);

    /// True if we're off the end of the list
    bool at_end() const;
};

#endif /* XAPIAN_INCLUDED_GLASS_SPELLINGWORDSLIST_H */
