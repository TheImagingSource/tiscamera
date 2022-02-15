
#pragma once

#include <tcamprop1.0_base/tcamprop_property_interface.h>
#include <tcam-property-1.0.h>
#include <gst-helper/gobject_ptr.h>

namespace tcamprop1_consumer::impl
{
    auto convert_GError_to_error_code_consumer( GError* err )->std::error_code;

    auto fetch_prop_static_info_str( TcamPropertyBase* node ) -> tcamprop1::prop_static_info_str;
    auto fetch_prop_state( TcamPropertyBase* node ) ->outcome::result<tcamprop1::prop_state>;

    template<class TDerived,class TItf>
    class consumer_prop_node_base : public TItf
    {
        TcamPropertyBase* get_prop_node() noexcept { return static_cast<TDerived*>( this )->get_derived_node(); }
    public:
        consumer_prop_node_base() = default;

        auto get_property_name() const noexcept -> std::string_view final { return static_info_.name; }

        auto get_property_info() const noexcept -> tcamprop1::prop_static_info final { return static_info_.to_prop_static_info(); }
        auto get_property_state( uint32_t /*flags*/ ) -> outcome::result<tcamprop1::prop_state> final
        {
            return fetch_prop_state( get_prop_node() );
        }
    protected:
        void    init( TcamPropertyBase* node )
        {
            static_info_ = fetch_prop_static_info_str( node );
        }
    private:
        tcamprop1::prop_static_info_str static_info_;
    };

    class prop_consumer_boolean : public consumer_prop_node_base<prop_consumer_boolean, tcamprop1::property_interface_boolean>
    {
    public:
        // CRTP stuff
        TcamPropertyBase* get_derived_node() noexcept { return TCAM_PROPERTY_BASE( ptr_.get() ); }
    public:
        prop_consumer_boolean( gobject_helper::gobject_ptr<TcamPropertyBoolean>&& ptr );

        auto get_property_default( uint32_t flags ) -> outcome::result<bool> final;

        auto get_property_value( uint32_t flags )->outcome::result<bool> final;
        auto set_property_value( bool value,uint32_t flags ) -> std::error_code final;
    private:
        gobject_helper::gobject_ptr<TcamPropertyBoolean>    ptr_;
    };

    class prop_consumer_integer : public consumer_prop_node_base<prop_consumer_integer,tcamprop1::property_interface_integer>
    {
    public:
        // CRTP stuff
        TcamPropertyBase* get_derived_node() noexcept { return TCAM_PROPERTY_BASE( ptr_.get() ); }
    public:
        prop_consumer_integer( gobject_helper::gobject_ptr<TcamPropertyInteger>&& ptr );

        auto get_property_range( uint32_t flags ) -> outcome::result<tcamprop1::prop_range_integer> final;
        auto get_property_default( uint32_t flags )->outcome::result<int64_t> final;

        auto get_property_value( uint32_t flags )->outcome::result<int64_t> final;
        auto set_property_value( int64_t value, uint32_t flags ) -> std::error_code final;

        auto get_representation() const noexcept -> tcamprop1::IntRepresentation_t final;
        auto get_unit() const noexcept -> std::string_view final;
    private:
        gobject_helper::gobject_ptr<TcamPropertyInteger>    ptr_;
    };

    class prop_consumer_float : public consumer_prop_node_base<prop_consumer_float, tcamprop1::property_interface_float>
    {
    public:
        // CRTP stuff
        TcamPropertyBase* get_derived_node() noexcept { return TCAM_PROPERTY_BASE( ptr_.get() ); }
    public:
        prop_consumer_float( gobject_helper::gobject_ptr<TcamPropertyFloat>&& ptr );
        
        auto get_property_range( uint32_t flags ) -> outcome::result<tcamprop1::prop_range_float> final;
        auto get_property_default( uint32_t flags )->outcome::result<double> final;

        auto get_property_value( uint32_t flags )->outcome::result<double> final;
        auto set_property_value( double value, uint32_t flags ) -> std::error_code final;

        auto get_representation() const noexcept -> tcamprop1::FloatRepresentation_t final;
        auto get_unit() const noexcept -> std::string_view final;
    private:
        gobject_helper::gobject_ptr<TcamPropertyFloat>    ptr_;
    };

    class prop_consumer_enumeration : public consumer_prop_node_base<prop_consumer_enumeration, tcamprop1::property_interface_enumeration>
    {
    public:
        // CRTP stuff
        TcamPropertyBase* get_derived_node() noexcept { return TCAM_PROPERTY_BASE( ptr_.get() ); }
    public:
        prop_consumer_enumeration( gobject_helper::gobject_ptr<TcamPropertyEnumeration>&& ptr );

        auto get_property_range( uint32_t flags ) -> outcome::result<tcamprop1::prop_range_enumeration> final;
        auto get_property_default( uint32_t flags )->outcome::result<std::string_view> final;

        auto get_property_value( uint32_t flags )->outcome::result<std::string_view> final;
        auto set_property_value( std::string_view value, uint32_t flags ) -> std::error_code final;
    private:
        gobject_helper::gobject_ptr<TcamPropertyEnumeration>    ptr_;
    };

    class prop_consumer_command : public consumer_prop_node_base<prop_consumer_command, tcamprop1::property_interface_command>
    {
    public:
        // CRTP stuff
        TcamPropertyBase* get_derived_node() noexcept { return TCAM_PROPERTY_BASE( ptr_.get() ); }
    public:
        prop_consumer_command( gobject_helper::gobject_ptr<TcamPropertyCommand>&& ptr );

        auto execute_command( uint32_t flags ) -> std::error_code final;
    private:
        gobject_helper::gobject_ptr<TcamPropertyCommand>    ptr_;
    };

    class prop_consumer_string :
        public consumer_prop_node_base<prop_consumer_string, tcamprop1::property_interface_string>
    {
    public:
        // CRTP stuff
        TcamPropertyBase* get_derived_node() noexcept
        {
            return TCAM_PROPERTY_BASE(ptr_.get());
        }

    public:
        prop_consumer_string(gobject_helper::gobject_ptr<TcamPropertyString>&& ptr);

        auto get_property_value(uint32_t flags = 0) -> outcome::result<std::string> final;
        auto set_property_value(std::string_view new_value, uint32_t flags = 0)
            -> std::error_code final;
    private:
        gobject_helper::gobject_ptr<TcamPropertyString> ptr_;
    };
    }

