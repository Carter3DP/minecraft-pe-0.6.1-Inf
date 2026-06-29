#ifndef COM_MOJANG_NBT__ByteTag_H__
#define COM_MOJANG_NBT__ByteTag_H__

//package com.mojang.nbt;

/* import java.io.* */

#include "Tag.h"
#include <string>

class ByteTag: public Tag
{
	typedef Tag super;
public:
    char data;

    ByteTag(const std::string& name)
	:	super(name)
	{}

    ByteTag(const std::string& name, char data)
	:	super(name),
		data(data)
	{
    }

    void write(IDataOutput* dos) /*throws IOException*/ override {
        dos->writeByte(data);
    }

    void load(IDataInput* dis) /*throws IOException*/ override {
        data = dis->readByte();
    }

    char getId() const override {
        return TAG_Byte;
    }

    std::string toString() const override {
        return std::string(data, 1);
    }

    //@Override
    bool equals(const Tag& rhs) const override {
        if (super::equals(rhs)) {
            return data == ((ByteTag&)rhs).data;
        }
        return false;
    }

    //@Override
    Tag* copy() const override {
        return new ByteTag(getName(), data);
    }
};

#endif /*COM_MOJANG_NBT__ByteTag_H__*/
