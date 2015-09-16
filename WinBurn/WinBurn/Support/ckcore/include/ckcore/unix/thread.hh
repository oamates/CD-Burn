/*
 * The ckCore library provides core software functionality.
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

/**
 * @file include/ckcore/unix/thread.hh
 * @brief Defines the Unix thread implementation.
 */
#pragma once
#include "ckcore/types.hh"

namespace ckcore
{
    namespace thread
	{
		bool create(tfunction func,void *param);
		bool sleep(ckcore::tuint32 milliseconds);

		/**
		 * @brief Thead mutex class.
		 */
		class Mutex
		{
		private:
            pthread_mutex_t mutex_;
            bool initialized_;
			bool locked_;

		public:
			Mutex();
			~Mutex();

			bool lock();
			bool unlock();
		};
	};
};

