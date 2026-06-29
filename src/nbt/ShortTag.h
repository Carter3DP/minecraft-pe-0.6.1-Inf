#ifndef COM_MOJANG_NBT__ShortTag_H__
#define COM_MOJANG_NBT__ShortTag_H__

//package com.mojang.nbt;

/* import java.io.* */

#include "Tag.h"
#include <sstream>

class ShortTag: public Tag {
	typedef Tag super;
public:
    short data;

    ShortTag(const std::string& name)
	:	super(name)
	{
    }

    ShortTag(const std::string& name, short data)
    :   super(name),
		data(data)
	{
    }

    void write(IDataOutput* dos) /*throws IOException*/ override {
        dos->writeShort(data);
    }

    void load(IDataInput* dis) /*throws IOException*/ override {
        data = dis->readShort();
    }

    char getId() const override {
        return TAG_Short;
    }

    std::string toString() const override {
        std::stringstream ss;
        ss << data;
        return ss.str();
    }

    //@Override
    Tag* copy() const override {
        return new ShortTag(getName(), data);
    }

    //@Override
    bool equals(const Tag& rhs) const override {
        if (super::equals(rhs)) {
            return data == ((ShortTag&)rhs).data;
        }
		return false;
    }
};

#endif /*COM_MOJANG_NBT__ShortTag_H__*/
