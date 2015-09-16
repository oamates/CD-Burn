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

#include <cxxtest/TestSuite.h>
#include <stdlib.h>
#include <algorithm>
#include "ckcore/types.hh"
#include "ckcore/filestream.hh"
#include "ckcore/bufferedstream.hh"
#include "ckcore/crcstream.hh"
#include "ckcore/nullstream.hh"
#include "ckcore/system.hh"
#include "ckcore/progress.hh"
#include "ckcore/progresser.hh"

class DummyProgress : public ckcore::Progress
{
public:
	void set_status(const ckcore::tchar *format,...) {}
	void notify(MessageType type,const ckcore::tchar *format,...) {}
	bool cancelled() { return false; }
};

class StreamTestSuite : public CxxTest::TestSuite
{
public:
    void testInStream()
    {
        ckcore::FileInStream is1(ckT("data/file/8253bytes"));
        ckcore::FileInStream fs(ckT("data/file/8253bytes"));
        ckcore::BufferedInStream is2(fs);

		// Test reading.
        for (int i = 0; i < 100; i++)
        {
            TS_ASSERT(is1.open());
            TS_ASSERT(fs.open());

            size_t buffer_size = (rand() % 2100) + 50;
            unsigned char *buffer1 = new unsigned char[buffer_size];
            unsigned char *buffer2 = new unsigned char[buffer_size];

            ckcore::tint64 read1 = 0,read2 = 0;
            while (!is1.end() && !is2.end())
            {
                ckcore::tint64 res1 = is1.read(buffer1,(ckcore::tuint32)buffer_size);
                ckcore::tint64 res2 = is2.read(buffer2,(ckcore::tuint32)buffer_size);

                TS_ASSERT(res1 != -1);
                TS_ASSERT(res2 != -1);

                TS_ASSERT_SAME_DATA(buffer1,buffer2,(unsigned int)buffer_size);

                read1 += res1;
                read2 += res2;
            }

            TS_ASSERT_EQUALS(is1.end(),is2.end());
            TS_ASSERT_EQUALS(read1,8253);
            TS_ASSERT_EQUALS(read2,8253);

            TS_ASSERT(is1.close());
            TS_ASSERT(fs.close());

            delete [] buffer1;
            delete [] buffer2;
        }

		// Test seeking.
		TS_ASSERT(is1.open());
        TS_ASSERT(fs.open());

		for (int i = 0; i < 100; i++)
        {
			// reset stream pointer.
			TS_ASSERT(is1.seek(0,ckcore::InStream::ckSTREAM_BEGIN));
			TS_ASSERT(is2.seek(0,ckcore::InStream::ckSTREAM_BEGIN));

            size_t buffer_size = (rand() % 2100) + 50;
            unsigned char *buffer1 = new unsigned char[buffer_size];
            unsigned char *buffer2 = new unsigned char[buffer_size];
			
			ckcore::tint64 read1 = 0,read2 = 0,count = 0;
            while (!is1.end() && !is2.end())
            {
				// After reading one time seek instead.
				if (count == 1)
				{
					TS_ASSERT(is1.seek((ckcore::tuint32)buffer_size,ckcore::InStream::ckSTREAM_CURRENT));
					TS_ASSERT(is2.seek((ckcore::tuint32)buffer_size,ckcore::InStream::ckSTREAM_CURRENT));
					read1 += buffer_size;
					read2 += buffer_size;
				}
				else
				{
					ckcore::tint64 res1 = is1.read(buffer1,(ckcore::tuint32)buffer_size);
					ckcore::tint64 res2 = is2.read(buffer2,(ckcore::tuint32)buffer_size);

					TS_ASSERT(res1 != -1);
					TS_ASSERT(res2 != -1);

					TS_ASSERT_SAME_DATA(buffer1,buffer2,(unsigned int)buffer_size);

					read1 += res1;
					read2 += res2;
				}

				count++;
            }

            TS_ASSERT_EQUALS(is1.end(),is2.end());
            TS_ASSERT_EQUALS(read1,8253);
            TS_ASSERT_EQUALS(read2,8253);

            delete [] buffer1;
            delete [] buffer2;
        }
    }

    void testOutStream()
    {
        ckcore::FileInStream is1(ckT("data/file/8253bytes"));
        ckcore::FileInStream is2(ckT("data/file/new"));
        ckcore::FileOutStream fs(ckT("data/file/new"));
        ckcore::BufferedOutStream os(fs);

        // Run 100 tests with different buffer sizes to capture buffer edge errors.
        for (int i = 0; i < 100; i++)
        {
            TS_ASSERT(is1.open());
            TS_ASSERT(fs.open());

            size_t buffer_size = (rand() % 2100) + 50;
            unsigned char *buffer1 = new unsigned char[buffer_size];
            unsigned char *buffer2 = new unsigned char[buffer_size];

            ckcore::tint64 written = 0;
            while (!is1.end())
            {
                ckcore::tint64 res1 = is1.read(buffer1,(ckcore::tuint32)buffer_size);
                TS_ASSERT(res1 != -1);

                ckcore::tint64 res2 = os.write(buffer1,(ckcore::tuint32)res1);
                TS_ASSERT(res2 != -1);

                TS_ASSERT_EQUALS(res1,res2);

                written += res2;
            }

            os.flush();

            TS_ASSERT_EQUALS(written,8253);

            TS_ASSERT(is1.close());
            TS_ASSERT(fs.close());

            // Perform the same operation as in the input stream test.
            TS_ASSERT(is1.open());
            TS_ASSERT(is2.open());

            ckcore::tint64 read1 = 0,read2 = 0;
            while (!is1.end() && !is2.end())
            {
                ckcore::tint64 res1 = is1.read(buffer1,(ckcore::tuint32)buffer_size);
                ckcore::tint64 res2 = is2.read(buffer2,(ckcore::tuint32)buffer_size);

                TS_ASSERT(res1 != -1);
                TS_ASSERT(res2 != -1);

                TS_ASSERT_SAME_DATA(buffer1,buffer2,(unsigned int)buffer_size);

                read1 += res1;
                read2 += res2;
            }

            TS_ASSERT_EQUALS(is1.end(),is2.end());
            TS_ASSERT_EQUALS(read1,8253);
            TS_ASSERT_EQUALS(read2,8253);

            TS_ASSERT(is1.close());
            TS_ASSERT(is2.close());

            TS_ASSERT(ckcore::File::remove(ckT("data/file/new")));

            delete [] buffer1;
            delete [] buffer2;
        }
    }

    void testCrcStream()
    {
        ckcore::FileInStream is1(ckT("data/file/8253bytes"));
        TS_ASSERT(is1.open());
        ckcore::FileInStream is2(ckT("data/file/123bytes"));
        TS_ASSERT(is2.open());
        ckcore::FileInStream is3(ckT("data/file/53bytes"));
        TS_ASSERT(is3.open());
        ckcore::FileInStream is4(ckT("data/file/0bytes"));
        TS_ASSERT(is4.open());

        // CRC-32.
        ckcore::CrcStream crc32(ckcore::CrcStream::ckCRC_32);

        ckcore::stream::copy(is1,crc32);
        TS_ASSERT_EQUALS(crc32.checksum(),0x33d5a2ec);
        crc32.reset();

        ckcore::stream::copy(is2,crc32);
        TS_ASSERT_EQUALS(crc32.checksum(),0xfa2e73f4);
        crc32.reset();

        ckcore::stream::copy(is3,crc32);
        TS_ASSERT_EQUALS(crc32.checksum(),0x30e06b16);
        crc32.reset();

        ckcore::stream::copy(is4,crc32);
        TS_ASSERT_EQUALS(crc32.checksum(),0x00000000);
        crc32.reset();

        is1.close();
        is1.open();
        is2.close();
        is2.open();
        is3.close();
        is3.open();
        is4.close();
        is4.open();

        // CRC-16 (CCITT polynomial).
        ckcore::CrcStream crc16udf(ckcore::CrcStream::ckCRC_CCITT);

        ckcore::stream::copy(is1,crc16udf);
        TS_ASSERT_EQUALS(crc16udf.checksum(),0x8430);
        crc16udf.reset();

        ckcore::stream::copy(is2,crc16udf);
        TS_ASSERT_EQUALS(crc16udf.checksum(),0x8bfe);
        crc16udf.reset();

        ckcore::stream::copy(is3,crc16udf);
        TS_ASSERT_EQUALS(crc16udf.checksum(),0xef2a);
        crc16udf.reset();

        ckcore::stream::copy(is4,crc16udf);
        TS_ASSERT_EQUALS(crc16udf.checksum(),0x0000);
        crc16udf.reset();

        // Test from the UDF 1.50 reference.
        unsigned char bytes[] = { 0x70, 0x6A, 0x77 };
        crc16udf.write(bytes,3);
        TS_ASSERT_EQUALS(crc16udf.checksum(),0x3299);

        is1.close();
        is1.open();
        is2.close();
        is2.open();
        is3.close();
        is3.open();
        is4.close();
        is4.open();

        // CRC-16 (IBM polynomial).
        ckcore::CrcStream crc16ibm(ckcore::CrcStream::ckCRC_16);

        ckcore::stream::copy(is1,crc16ibm);
        TS_ASSERT_EQUALS(crc16ibm.checksum(),0x398e);
        crc16ibm.reset();

        ckcore::stream::copy(is2,crc16ibm);
        TS_ASSERT_EQUALS(crc16ibm.checksum(),0xd3bb);
        crc16ibm.reset();

        ckcore::stream::copy(is3,crc16ibm);
        TS_ASSERT_EQUALS(crc16ibm.checksum(),0xb7d4);
        crc16ibm.reset();

        ckcore::stream::copy(is4,crc16ibm);
        TS_ASSERT_EQUALS(crc16ibm.checksum(),0x0000);
        crc16ibm.reset();
    }

    void testNullStream()
    {
        ckcore::NullStream ns;
        ns.write(NULL,42);
        TS_ASSERT_EQUALS(ns.written(),42);
        
        ns.write(NULL,1);
        TS_ASSERT_EQUALS(ns.written(),43);
        
        ns.write(NULL,754);
        TS_ASSERT_EQUALS(ns.written(),797);
    }

	void testCopy()
	{
		ckcore::FileInStream is1(ckT("data/file/8253bytes"));
        TS_ASSERT(is1.open());
        ckcore::FileInStream is2(ckT("data/file/53bytes"));
        TS_ASSERT(is2.open());

		ckcore::NullStream ns1,ns2,ns3,ns4;

		DummyProgress dp;
		ckcore::Progresser p(dp,0xffffffff);

		TS_ASSERT(ckcore::stream::copy(is1,ns1,p,825));
		TS_ASSERT(ckcore::stream::copy(is2,ns2,p,825));

		TS_ASSERT_EQUALS(ns1.written(),825);
		TS_ASSERT_EQUALS(ns2.written(),825);

		// Try again reaching outside the internal buffer used within the copy
		// function.
		TS_ASSERT(is1.seek(0,ckcore::InStream::ckSTREAM_BEGIN));
		TS_ASSERT(ckcore::stream::copy(is1,ns3,p,8200));
		TS_ASSERT_EQUALS(ns3.written(),8200);

		// Try again reaching outside the internal buffer used within the copy
		// function and also reaching outside the stream.
		TS_ASSERT(is1.seek(0,ckcore::InStream::ckSTREAM_BEGIN));
		TS_ASSERT(ckcore::stream::copy(is1,ns4,p,9200));
		TS_ASSERT_EQUALS(ns4.written(),9200);
	}
};

