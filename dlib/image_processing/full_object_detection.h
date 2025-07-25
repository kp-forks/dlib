// Copyright (C) 2012  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#ifndef DLIB_FULL_OBJECT_DeTECTION_Hh_
#define DLIB_FULL_OBJECT_DeTECTION_Hh_

#include "../geometry.h"
#include "full_object_detection_abstract.h"
#include <vector>
#include "../serialize.h"

namespace dlib
{

// ----------------------------------------------------------------------------------------

    const static dpoint OBJECT_PART_NOT_PRESENT(0x7FFFFFFFFFFFF,
                                                0x7FFFFFFFFFFFF);

// ----------------------------------------------------------------------------------------

    class full_object_detection
    {
    public:
        full_object_detection(
            const rectangle& rect_,
            const std::vector<dpoint>& parts_
        ) : rect(rect_), parts(parts_) {}

        full_object_detection(
            const rectangle& rect_,
            const std::vector<point>& parts_
        ) : rect(rect_), parts (parts_.begin(), parts_.end())
        {}

        full_object_detection(){}

        explicit full_object_detection(
            const rectangle& rect_
        ) : rect(rect_) {}

        const rectangle& get_rect() const { return rect; }
        rectangle& get_rect() { return rect; }
        unsigned long num_parts() const { return parts.size(); }

        const dpoint& part(
            unsigned long idx
        ) const 
        { 
            // make sure requires clause is not broken
            DLIB_ASSERT(idx < num_parts(),
                "\t dpoint full_object_detection::part()"
                << "\n\t Invalid inputs were given to this function "
                << "\n\t idx:         " << idx  
                << "\n\t num_parts(): " << num_parts()  
                << "\n\t this:        " << this
                );
            return parts[idx]; 
        }

        dpoint& part(
            unsigned long idx
        )  
        { 
            // make sure requires clause is not broken
            DLIB_ASSERT(idx < num_parts(),
                "\t dpoint full_object_detection::part()"
                << "\n\t Invalid inputs were given to this function "
                << "\n\t idx:         " << idx  
                << "\n\t num_parts(): " << num_parts()  
                << "\n\t this:        " << this
                );
            return parts[idx]; 
        }

        friend void serialize (
            const full_object_detection& item,
            std::ostream& out
        )
        {
            int version = 2;
            serialize(version, out);
            serialize(item.rect, out);
            serialize(item.parts, out);
        }

        friend void deserialize (
            full_object_detection& item,
            std::istream& in
        )
        {
            int version = 0;
            deserialize(version, in);

            if (version != 1 && version != 2)
                throw serialization_error("Unexpected version encountered while deserializing dlib::full_object_detection.");

            deserialize(item.rect, in);

            // Legacy support: read vector<point, 2> and cast to vector<dpoint, 2>
            if (version == 1) 
            {
                std::vector<point> legacy_parts;
                deserialize(legacy_parts, in);
                item.parts = std::vector<dpoint>(legacy_parts.begin(), legacy_parts.end());
            }
            else 
            { 
                // version 2 - deserialize into vector<dpoint, 2>
                deserialize(item.parts, in);
            }

        }

        bool operator==(
            const full_object_detection& rhs
        ) const
        {
            if (rect != rhs.rect)
                return false;
            if (parts.size() != rhs.parts.size())
                return false;
            for (size_t i = 0; i < parts.size(); ++i)
            {
                if (parts[i] != rhs.parts[i])
                    return false;
            }
            return true;
        }

    private:
        rectangle rect;
        std::vector<dpoint> parts;  
    };

// ----------------------------------------------------------------------------------------

    inline bool all_parts_in_rect (
        const full_object_detection& obj
    )
    {
        for (unsigned long i = 0; i < obj.num_parts(); ++i)
        {
            if (obj.get_rect().contains(obj.part(i)) == false &&
                obj.part(i) != OBJECT_PART_NOT_PRESENT)
                return false;
        }
        return true;
    }

// ----------------------------------------------------------------------------------------

    struct mmod_rect
    {
        mmod_rect() = default; 
        mmod_rect(const rectangle& r) : rect(r) {}
        mmod_rect(const rectangle& r, double score) : rect(r),detection_confidence(score) {}
        mmod_rect(const rectangle& r, double score, const std::string& label) : rect(r),detection_confidence(score), label(label) {}

        rectangle rect;
        double detection_confidence = 0;
        bool ignore = false;
        std::string label;

        operator rectangle() const { return rect; }
        bool operator == (const mmod_rect& rhs) const
        { 
            return rect == rhs.rect 
                   && detection_confidence == rhs.detection_confidence
                   && ignore == rhs.ignore 
                   && label == rhs.label;
        }
    };

    inline mmod_rect ignored_mmod_rect(const rectangle& r)
    {
        mmod_rect temp(r);
        temp.ignore = true;
        return temp;
    }

    inline void serialize(const mmod_rect& item, std::ostream& out)
    {
        int version = 2;
        serialize(version, out);
        serialize(item.rect, out);
        serialize(item.detection_confidence, out);
        serialize(item.ignore, out);
        serialize(item.label, out);
    }

    inline void deserialize(mmod_rect& item, std::istream& in)
    {
        int version = 0;
        deserialize(version, in);
        if (version != 1 && version != 2)
            throw serialization_error("Unexpected version found while deserializing dlib::mmod_rect");
        deserialize(item.rect, in);
        deserialize(item.detection_confidence, in);
        deserialize(item.ignore, in);
        if (version == 2)
            deserialize(item.label, in);
        else
            item.label = "";
    }

// ----------------------------------------------------------------------------------------

    struct yolo_rect
    {
        yolo_rect() = default;
        yolo_rect(const drectangle& r) : rect(r) {}
        yolo_rect(const drectangle& r, double score) : rect(r),detection_confidence(score) {}
        yolo_rect(const drectangle& r, double score, const std::string& label) : rect(r),detection_confidence(score), label(label) {}
        yolo_rect(const mmod_rect& r) : rect(r.rect), detection_confidence(r.detection_confidence), ignore(r.ignore), label(r.label) {}

        drectangle rect;
        double detection_confidence = 0;
        bool ignore = false;
        std::string label;
        std::vector<std::pair<double, std::string>> labels;

        operator rectangle() const { return rect; }
        bool operator == (const yolo_rect& rhs) const
        {
            return rect == rhs.rect
                   && detection_confidence == rhs.detection_confidence
                   && ignore == rhs.ignore
                   && label == rhs.label;
        }
        bool operator<(const yolo_rect& rhs) const
        {
            return detection_confidence < rhs.detection_confidence;
        }
    };

    inline void serialize(const yolo_rect& item, std::ostream& out)
    {
        int version = 1;
        serialize(version, out);
        serialize(item.rect, out);
        serialize(item.detection_confidence, out);
        serialize(item.ignore, out);
        serialize(item.label, out);
        serialize(item.labels, out);
    }

    inline void deserialize(yolo_rect& item, std::istream& in)
    {
        int version = 0;
        deserialize(version, in);
        if (version != 1)
            throw serialization_error("Unexpected version found while deserializing dlib::yolo_rect");
        deserialize(item.rect, in);
        deserialize(item.detection_confidence, in);
        deserialize(item.ignore, in);
        deserialize(item.label, in);
        deserialize(item.labels, in);
    }

// ----------------------------------------------------------------------------------------

}

#endif // DLIB_FULL_OBJECT_DeTECTION_H_

