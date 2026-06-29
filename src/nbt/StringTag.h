#ifndef COM_MOJANG_NBT__StringTag_H__
#define COM_MOJANG_NBT__StringTag_H__

//package com.mojang.nbt;

#include "Tag.h"
#include <sstream>

class StringTag: public Tag
{
	typedef Tag super;
public:
    std::string data;
	int len;

    StringTag(const std::string& name)
	:	super(name),
		len(name.length())
	{
    }

    StringTag(const std::string& name, const std::string& data)
	:	super(name),
		data(data),
		len(name.length())
	{
    }

    void write(IDataOutput* dos) /*throws IOException*/ override {
        dos->writeString(data);
    }

    void load(IDataInput* dis) /*throws IOException*/ override {
		data = dis->readString(); // just a tiny bit slower, but safer
    }

    char getId() const override {
        return TAG_String;
    }

    std::string toString() const override {
        std::stringstream ss;
        ss << data;
        return ss.str();
    }

    //@Override
    Tag* copy() const override {
        return new StringTag(getName(), data);
    }

    //@Override
    bool equals(const Tag& rhs) const override {
        if (super::equals(rhs)) {
            StringTag& o = (StringTag&) rhs;
			return data == o.data;
        }
        return false;
    }
};

#endif /*COM_MOJANG_NBT__StringTag_H__*/
