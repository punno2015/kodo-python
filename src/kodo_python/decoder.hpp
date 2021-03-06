// Copyright Steinwurf ApS 2011-2013.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <Python.h>
#include <bytesobject.h>

#include <boost/python/args.hpp>

#include <kodo/has_partial_decoding_tracker.hpp>
#include <kodo/is_partial_complete.hpp>
#include <kodo/write_feedback.hpp>

#include <sak/storage.hpp>

#include "has_recode.hpp"
#include "coder.hpp"
#include "resolve_field_name.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace kodo_python
{

    template<class Decoder>
    PyObject* copy_symbols(Decoder& decoder)
    {
        std::vector<uint8_t> payload(decoder.block_size());
        auto storage = sak::mutable_storage(
            payload.data(), decoder.block_size());
        decoder.copy_symbols(storage);
        #if PY_MAJOR_VERSION >= 3
        return PyBytes_FromStringAndSize(
            (char*)payload.data(), decoder.block_size());
        #else
        return PyString_FromStringAndSize(
            (char*)payload.data(), decoder.block_size());
        #endif
    }

    template<class Decoder>
    PyObject* recode(Decoder& decoder)
    {
        std::vector<uint8_t> payload(decoder.payload_size());
        auto length = decoder.recode(payload.data());

        #if PY_MAJOR_VERSION >= 3
        return PyBytes_FromStringAndSize((char*)payload.data(), length);
        #else
        return PyString_FromStringAndSize((char*)payload.data(), length);
        #endif
    }

    template<class Decoder>
    void decode(Decoder& decoder, const std::string& data)
    {
        std::vector<uint8_t> payload(data.length());
        std::copy(data.c_str(), data.c_str() + data.length(), payload.data());
        decoder.decode(payload.data());
    }

    template<class Decoder>
    bool is_partial_complete(Decoder& decoder)
    {
        return kodo::is_partial_complete(decoder);
    }

    template<class Decoder>
    PyObject* write_feedback(Decoder& decoder)
    {
        std::vector<uint8_t> payload(decoder.feedback_size());
        uint32_t length = kodo::write_feedback(decoder, payload.data());
        #if PY_MAJOR_VERSION >= 3
        return PyBytes_FromStringAndSize((char*)payload.data(), length);
        #else
        return PyString_FromStringAndSize((char*)payload.data(), length);
        #endif
    }

    template<bool HAS_PARTIAL_DECODING_TRACKER, class Type>
    struct is_partial_complete_method
    {
        template<class DecoderClass>
        is_partial_complete_method(DecoderClass& decoder_class)
        {
            (void) decoder_class;
        }
    };

    template<class Type>
    struct is_partial_complete_method<true, Type>
    {
        template<class DecoderClass>
        is_partial_complete_method(DecoderClass& decoder_class)
        {
            decoder_class
            .def("is_partial_complete", &is_partial_complete<Type>,
                "Check whether the decoding matrix is partially decoded.\n\n"
                "\t:returns: True if the decoding matrix is partially "
                "decoded.\n");
        }
    };

    template<bool HAS_RECODE, class Type>
    struct recode_method
    {
        template<class DecoderClass>
        recode_method(DecoderClass& decoder_class)
        {
            (void) decoder_class;
        }
    };

    template<class Type>
    struct recode_method<true, Type>
    {
        template<class DecoderClass>
        recode_method(DecoderClass& decoder_class)
        {
            decoder_class
            .def("recode", &recode<Type>,
                "Recode symbol.\n\n"
                "\t:returns: The recoded symbol.\n"
            );
        }
    };

    template<template<class, class> class Coder, class Type>
    struct extra_decoder_methods
    {
        template<class DecoderClass>
        extra_decoder_methods(DecoderClass& decoder_class)
        {
            (void) decoder_class;
        }
    };

    template<class Type>
    struct extra_decoder_methods<kodo::rlnc::sliding_window_decoder, Type>
    {
        template<class DecoderClass>
        extra_decoder_methods(DecoderClass& decoder_class)
        {
            decoder_class
            .def("feedback_size", &Type::feedback_size,
                "Return the required feedback buffer size in bytes.\n\n"
                "\t:returns: The required feedback buffer size in bytes.\n"
            )
            .def("write_feedback", &write_feedback<Type>,
                "Return a buffer containing the feedback.\n\n"
                "\t:returns: A buffer containing the feedback.\n");
        }
    };

    template<template<class, class> class Coder, class Field, class TraceTag>
    void decoder(const std::string& stack)
    {
        using boost::python::arg;
        using decoder_type = Coder<Field, TraceTag>;

        std::string field = resolve_field_name<Field>();
        std::string kind = "Decoder";
        std::string trace = kodo::has_trace<decoder_type>::value ? "Trace" : "";
        std::string name = stack + kind + field + trace;

        auto decoder_class = coder<Coder, Field, TraceTag>(name)
        .def("decode", &decode<decoder_type>, arg("symbol_data"),
            "Decode the provided encoded symbol.\n\n"
            "\t:param symbol_data: The encoded symbol.\n"
        )
        .def("is_complete", &decoder_type::is_complete,
            "Check whether decoding is complete.\n\n"
            "\t:returns: True if the decoding is complete.\n"
        )
        .def("symbols_uncoded", &decoder_type::symbols_uncoded,
            "Return the number of uncoded symbols.\n\n"
            "\t:returns: The number of symbols which have been uncoded.\n"
        )
        .def("copy_symbols", &copy_symbols<decoder_type>,
            "Return the decoded symbols.\n\n"
            "\t:returns: The decoded symbols.\n"
        );

        (recode_method<has_recode<decoder_type>::value, decoder_type>
            (decoder_class));

        (is_partial_complete_method<
            kodo::has_partial_decoding_tracker<decoder_type>::value,
            decoder_type> (decoder_class));

        (extra_decoder_methods<Coder, decoder_type>(decoder_class));
    }
}


