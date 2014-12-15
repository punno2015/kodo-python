// Copyright Steinwurf ApS 2011-2013.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#include <boost/python.hpp>
#include <boost/python/docstring_options.hpp>

#include <fifi/binary.hpp>
#include <fifi/binary4.hpp>
#include <fifi/binary8.hpp>
#include <fifi/binary16.hpp>

#include <kodo/rlnc/full_vector_codes.hpp>
#include <kodo/rlnc/on_the_fly_codes.hpp>
#include <kodo/rlnc/sliding_window_decoder.hpp>
#include <kodo/rlnc/sliding_window_encoder.hpp>
#include <kodo/rlnc/sparse_full_vector_encoder.hpp>

#include <kodo/nocode/carousel_decoder.hpp>
#include <kodo/nocode/carousel_encoder.hpp>

#include <kodo/disable_trace.hpp>
#include <kodo/enable_trace.hpp>

#include <kodo/pool_factory.hpp>

#include "decoder.hpp"
#include "encoder.hpp"
#include "factory.hpp"
#include "has_encode.hpp"
#include "resolve_field_name.hpp"

#include <string>

namespace kodo_python
{
    template<template<class, class> class Coder, class Field, class TraceTag,
        bool IsEncoder>
    struct create_coder;

    template<template<class, class> class Coder, class Field, class TraceTag>
    struct create_coder<Coder, Field, TraceTag, true>
    {
        create_coder(const std::string& stack)
        {
            encoder<Coder, Field, TraceTag>(stack);
        }
    };

    template<template<class, class> class Coder, class Field, class TraceTag>
    struct create_coder<Coder, Field, TraceTag, false>
    {
        create_coder(const std::string& stack)
        {
            decoder<Coder, Field, TraceTag>(stack);
        }
    };

    template< template<class, class> class Coder, class Field, class TraceTag>
    void create(const std::string& stack)
    {
        factory<Coder, Field, TraceTag>(stack);
        create_coder<Coder, Field, TraceTag,
            has_encode<Coder<Field, TraceTag>>::value>coder(stack);
    }

    template<template<class, class> class Coder, class Field>
    void create_trace(const std::string& stack)
    {
        create<Coder, Field, kodo::disable_trace>(stack);
        create<Coder, Field, kodo::enable_trace>(stack);
    }

    template<template<class, class> class Coder>
    void create_field(const std::string& stack)
    {
        create_trace<Coder, fifi::binary>(stack);
        create_trace<Coder, fifi::binary4>(stack);
        create_trace<Coder, fifi::binary8>(stack);
        create_trace<Coder, fifi::binary16>(stack);
    }

    template<class Field, class TraceTag>
    class carousel_decoder_wrapper : public
        kodo::nocode::carousel_decoder<TraceTag>
    {
    public:
        using factory = kodo::pool_factory<carousel_decoder_wrapper>;

    public:
        uint32_t symbols_uncoded()
        {
            return kodo::nocode::carousel_decoder<TraceTag>::rank();
        }
    };

    template<class Field, class TraceTag>
    class carousel_encoder_wrapper : public
        kodo::nocode::carousel_encoder<TraceTag>
    {
    public:
        using factory = kodo::pool_factory<carousel_encoder_wrapper>;
    };

    void create_stacks()
    {
        using namespace kodo;

        create_field<rlnc::full_vector_encoder>("FullVector");
        create_field<rlnc::full_vector_decoder>("FullVector");

        create_field<rlnc::sparse_full_vector_encoder>("SparseFullVector");

        create_field<rlnc::on_the_fly_encoder>("OnTheFly");
        create_field<rlnc::on_the_fly_decoder>("OnTheFly");

        create_field<rlnc::sliding_window_encoder>("SlidingWindow");
        create_field<rlnc::sliding_window_decoder>("SlidingWindow");

        create_trace<carousel_decoder_wrapper, no_field>("NoCode");
        create_trace<carousel_encoder_wrapper, no_field>("NoCode");
    }

    BOOST_PYTHON_MODULE(kodo)
    {
        boost::python::docstring_options doc_options;
        doc_options.disable_signatures();
        create_stacks();
    }
}
