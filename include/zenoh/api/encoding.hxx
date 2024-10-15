//
// Copyright (c) 2024 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>

#pragma once
#include <string>

#include "../zenohc.hxx"
#include "base.hxx"
#include "interop.hxx"

namespace zenoh {

/// @brief The <a href="https://zenoh.io/docs/manual/abstractions/#encoding"> encoding </a> of Zenoh data.
class Encoding : public Owned<::z_owned_encoding_t> {
   public:
    /// @name Constructors

    /// @brief Construct default encoding.
    Encoding() : Owned(nullptr) {}

    /// @brief Construct encoding from string.
    Encoding(std::string_view s, ZResult* err = nullptr) : Owned(nullptr) {
        __ZENOH_RESULT_CHECK(::z_encoding_from_substr(&this->_0, s.data(), s.size()), err,
                             std::string("Failed to create encoding from ").append(s));
    }

    /// @brief Copy constructor.
    Encoding(const Encoding& other) : Encoding() { ::z_encoding_clone(&this->_0, interop::as_loaned_c_ptr(other)); };

    Encoding(Encoding&& other) = default;

    /// @name Methods

    /// @brief Get string representation of encoding.
    std::string as_string() const {
        ::z_owned_string_t s;
        ::z_encoding_to_string(interop::as_loaned_c_ptr(*this), &s);
        std::string out = std::string(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return out;
    }

    /// @brief Set a schema to this encoding from a string.
    ///
    /// Zenoh does not define what a schema is and its semantics is left to the implementer.
    /// E.g. a common schema for `text/plain` encoding is `utf-8`.
    void set_schema(std::string_view schema, ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(
            ::z_encoding_set_schema_from_substr(interop::as_loaned_c_ptr(*this), schema.data(), schema.size()), err,
            "Failed to set encoding schema");
    }

    /// @name Operators

    /// @brief Assignment operator.
    Encoding& operator=(const Encoding& other) {
        if (this != &other) {
            ::z_drop(z_move(this->_0));
            ::z_encoding_clone(&this->_0, interop::as_loaned_c_ptr(other));
        }
        return *this;
    };

    /// @brief Equality relation.
    /// @param other an encoding to compare with.
    /// @return ``true`` if both encodings are equal, ``false`` otherwise.
    bool operator==(const Encoding& other) const {
        return ::z_encoding_equals(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(other));
    };

    Encoding& operator=(Encoding&& other) = default;

#if defined(ZENOHCXX_ZENOHC) || (Z_FEATURE_ENCODING_VALUES == 1)
    struct Predefined {
        /// @name Methods

        /// Just some bytes.
        /// Constant alias for string: `"zenoh/bytes"`.
        /// This encoding supposes that the payload was constructed directly from `std::vector<uint8_t>` and
        /// its data can be accessed via `Bytes::as_vector`.
        static const Encoding& zenoh_bytes() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_zenoh_bytes());
        };

        /// A UTF-8 string.
        /// Constant alias for string: `"zenoh/string"`.
        /// This encoding supposes that the payload was created directly from `std::string`, `std::string_view` or a
        /// null-terminated string and its data can be accessed via `Bytes::as_string`.
        static const Encoding& zenoh_string() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_zenoh_string());
        };

        /// Zenoh serialized data.
        ///
        /// Constant alias for string: `"zenoh/serialized"`.
        ///
        /// This encoding supposes that the payload was created using ``zenoh::ext::serialize`` or with the help of
        /// ``zenoh::ext::Serializer`` and its data can be accessed via ``zenoh::ext::deserialize`` or with the help of
        /// ``zenoh::ext::Deserializer``.
        static const Encoding& zenoh_serialized() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_zenoh_serialized());
        };

        /// An application-specific stream of bytes.
        /// Constant alias for string: `"application/octet-stream"`.
        static const Encoding& application_octet_stream() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_octet_stream());
        };

        /// A textual file.
        /// Constant alias for string: `"text/plain"`.
        static const Encoding& text_plain() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_text_plain()); };

        /// JSON data intended to be consumed by an application.
        /// Constant alias for string: `"application/json"`.
        static const Encoding& application_json() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_json());
        };

        /// JSON data intended to be human readable.
        /// Constant alias for string: `"text/json"`.
        static const Encoding& text_json() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_text_json()); };

        /// A Common Data Representation (CDR)-encoded data.
        /// Constant alias for string: `"application/cdr"`.
        static const Encoding& application_cdr() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_cdr());
        };

        /// A Concise Binary Object Representation (CBOR)-encoded data.
        /// Constant alias for string: `"application/cbor"`.
        static const Encoding& application_cbor() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_cbor());
        };

        /// YAML data intended to be consumed by an application.
        /// Constant alias for string: `"application/yaml"`.
        static const Encoding& application_yaml() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_yaml());
        };

        /// YAML data intended to be human readable.
        /// Constant alias for string: `"text/yaml"`.
        static const Encoding& text_yaml() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_text_yaml()); };

        /// JSON5 encoded data that are human readable.
        /// Constant alias for string: `"text/json5"`.
        static const Encoding& text_json5() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_text_json5()); };

        /// A Python object serialized using [pickle](https://docs.python.org/3/library/pickle.html).
        /// Constant alias for string: `"application/python-serialized-object"`.
        static const Encoding& application_python_serialized_object() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_python_serialized_object());
        };

        /// An application-specific protobuf-encoded data.
        /// Constant alias for string: `"application/protobuf"`.
        static const Encoding& application_protobuf() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_protobuf());
        };

        /// A Java serialized object.
        /// Constant alias for string: `"application/java-serialized-object"`.
        static const Encoding& application_java_serialized_object() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_java_serialized_object());
        };

        /// An [openmetrics](https://github.com/OpenObservability/OpenMetrics) data, commonly used by
        /// [Prometheus](https://prometheus.io/).
        /// Constant alias for string: `"application/openmetrics-text"`.
        static const Encoding& application_openmetrics_text() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_openmetrics_text());
        };

        /// A Portable Network Graphics (PNG) image.
        /// Constant alias for string: `"image/png"`.
        static const Encoding& image_png() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_image_png()); };

        /// A Joint Photographic Experts Group (JPEG) image.
        /// Constant alias for string: `"image/jpeg"`.
        static const Encoding& image_jpeg() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_image_jpeg()); };

        /// A Graphics Interchange Format (GIF) image.
        /// Constant alias for string: `"image/gif"`.
        static const Encoding& image_gif() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_image_gif()); };

        /// A BitMap (BMP) image.
        /// Constant alias for string: `"image/bmp"`.
        static const Encoding& image_bmp() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_image_bmp()); };

        /// A Web Portable (WebP) image.
        /// Constant alias for string: `"image/webp"`.
        static const Encoding& image_webp() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_image_webp()); };

        /// An XML file intended to be consumed by an application.
        /// Constant alias for string: `"application/xml"`.
        static const Encoding& application_xml() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_xml());
        };

        /// An encoded list of tuples, each consisting of a name and a value.
        /// Constant alias for string: `"application/x-www-form-urlencoded"`.
        static const Encoding& application_x_www_form_urlencoded() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_x_www_form_urlencoded());
        };

        /// An HTML file.
        /// Constant alias for string: `"text/html"`.
        static const Encoding& text_html() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_text_html()); };

        /// An XML file that is human-readable.
        /// Constant alias for string: `"text/xml"`.
        static const Encoding& text_xml() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_text_xml()); };

        /// A CSS file.
        /// Constant alias for string: `"text/css"`.
        static const Encoding& text_css() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_text_css()); };

        /// A JavaScript file.
        /// Constant alias for string: `"text/javascript"`.
        static const Encoding& text_javascript() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_text_javascript());
        };

        /// A Markdown file.
        /// Constant alias for string: `"text/markdown"`.
        static const Encoding& text_markdown() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_text_markdown());
        };

        /// A CSV file.
        /// Constant alias for string: `"text/csv"`.
        static const Encoding& text_csv() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_text_csv()); };

        /// An application-specific SQL query.
        /// Constant alias for string: `"application/sql"`.
        static const Encoding& application_sql() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_sql());
        };

        /// Constrained Application Protocol (CoAP) data intended for CoAP-to-HTTP and HTTP-to-CoAP proxies.
        /// Constant alias for string: `"application/coap-payload"`.
        static const Encoding& application_coap_payload() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_coap_payload());
        };

        /// Defines a JSON document structure for expressing a sequence of operations to apply to a JSON document.
        /// Constant alias for string: `"application/json-patch+json"`.
        static const Encoding& application_json_patch_json() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_json_patch_json());
        };

        /// A JSON text sequence consists of any number of JSON texts, all encoded in UTF-8.
        /// Constant alias for string: `"application/json-seq"`.
        static const Encoding& application_json_seq() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_json_seq());
        };

        /// A JSONPath defines a string syntax for selecting and extracting JSON values from within a given JSON value.
        /// Constant alias for string: `"application/jsonpath"`.
        static const Encoding& application_jsonpath() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_jsonpath());
        };

        /// A JSON Web Token (JWT).
        /// Constant alias for string: `"application/jwt"`.
        static const Encoding& application_jwt() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_jwt());
        };

        /// An application-specific MPEG-4 encoded data, either audio or video.
        /// Constant alias for string: `"application/mp4"`.
        static const Encoding& application_mp4() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_mp4());
        };

        /// A SOAP 1.2 message serialized as XML 1.0.
        /// Constant alias for string: `"application/soap+xml"`.
        static const Encoding& application_soap_xml() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_soap_xml());
        };

        /// A YANG-encoded data commonly used by the Network Configuration Protocol (NETCONF).
        /// Constant alias for string: `"application/yang"`.
        static const Encoding& application_yang() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_application_yang());
        };

        /// A MPEG-4 Advanced Audio Coding (AAC) media.
        /// Constant alias for string: `"audio/aac"`.
        static const Encoding& audio_aac() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_audio_aac()); };

        /// A Free Lossless Audio Codec (FLAC) media.
        /// Constant alias for string: `"audio/flac"`.
        static const Encoding& audio_flac() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_audio_flac()); };

        /// An audio codec defined in MPEG-1, MPEG-2, MPEG-4, or registered at the MP4 registration authority.
        /// Constant alias for string: `"audio/mp4"`.
        static const Encoding& audio_mp4() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_audio_mp4()); };

        /// An Ogg-encapsulated audio stream.
        /// Constant alias for string: `"audio/ogg"`.
        static const Encoding& audio_ogg() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_audio_ogg()); };

        /// A Vorbis-encoded audio stream.
        /// Constant alias for string: `"audio/vorbis"`.
        static const Encoding& audio_vorbis() {
            return interop::as_owned_cpp_ref<Encoding>(::z_encoding_audio_vorbis());
        };

        /// A h261-encoded video stream.
        /// Constant alias for string: `"video/h261"`.
        static const Encoding& video_h261() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_video_h261()); };

        /// A h263-encoded video stream.
        /// Constant alias for string: `"video/h263"`.
        static const Encoding& video_h263() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_video_h263()); };

        /// A h264-encoded video stream.
        /// Constant alias for string: `"video/h264"`.
        static const Encoding& video_h264() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_video_h264()); };

        /// A h265-encoded video stream.
        /// Constant alias for string: `"video/h265"`.
        static const Encoding& video_h265() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_video_h265()); };

        /// A h266-encoded video stream.
        /// Constant alias for string: `"video/h266"`.
        static const Encoding& video_h266() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_video_h266()); };

        /// A video codec defined in MPEG-1, MPEG-2, MPEG-4, or registered at the MP4 registration authority.
        /// Constant alias for string: `"video/mp4"`.
        static const Encoding& video_mp4() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_video_mp4()); };

        /// An Ogg-encapsulated video stream.
        /// Constant alias for string: `"video/ogg"`.
        static const Encoding& video_ogg() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_video_ogg()); };

        /// An uncompressed, studio-quality video stream.
        /// Constant alias for string: `"video/raw"`.
        static const Encoding& video_raw() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_video_raw()); };

        /// A VP8-encoded video stream.
        /// Constant alias for string: `"video/vp8"`.
        static const Encoding& video_vp8() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_video_vp8()); };

        /// A VP9-encoded video stream.
        /// Constant alias for string: `"video/vp9"`.
        static const Encoding& video_vp9() { return interop::as_owned_cpp_ref<Encoding>(::z_encoding_video_vp9()); };
    };
#endif
};
}  // namespace zenoh