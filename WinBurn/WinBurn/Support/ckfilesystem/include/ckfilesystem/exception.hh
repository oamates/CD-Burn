/*
 * The ckFileSystem library provides file system functionality.
 * Copyright (C) 2006-2009 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <ckcore/types.hh>
#include <ckcore/exception.hh>

namespace ckfilesystem
{
	/**
	 * @brief Class for file exceptions.
	 */
	class FileOpenException : public ckcore::Exception
	{
	private:
		ckcore::tstring file_path_;

	public:
		FileOpenException(const ckcore::tstring &file_path) :
			file_path_(file_path)
		{
			message_  = ckT("Unable to open file \"");
			message_ += file_path.c_str();
			message_ += ckT("\" for read/write access.");
		}

		virtual ~FileOpenException() {};

		const ckcore::tstring &file_path() const
		{
			return file_path_;
		}
	};
};
