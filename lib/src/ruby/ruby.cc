#include <facter/ruby/ruby.hpp>
#include <facter/logging/logging.hpp>
#include <internal/ruby/module.hpp>
#include <leatherman/ruby/api.hpp>
#include <leatherman/logging/logging.hpp>

using namespace std;
using namespace facter::facts;
using namespace leatherman::ruby;

static const char load_puppet[] =
"require 'puppet'\n"
"Puppet.initialize_settings\n"
"unless $LOAD_PATH.include?(Puppet[:libdir])\n"
"    $LOAD_PATH << Puppet[:libdir]\n"
"end\n"
"Facter.reset\n"
"Facter.search_external([Puppet[:pluginfactdest]])";

namespace facter { namespace ruby {

    bool initialize(bool include_stack_trace)
    {
#ifdef FACTER_RUBY
        api::ruby_lib_location = FACTER_RUBY;
#endif
        try {
            auto& ruby = api::instance();
            ruby.initialize();
            ruby.include_stack_trace(include_stack_trace);
        } catch (runtime_error& ex) {
            LOG_WARNING("%1%: facts requiring Ruby will not be resolved.", ex.what());
            return false;
        }
        return true;
    }

    void load_custom_facts(collection& facts, bool initialize_puppet, vector<string> const& paths)
    {
        api& ruby = api::instance();
        module mod(facts, {}, !initialize_puppet);
        if (initialize_puppet) {
            try {
                ruby.eval(load_puppet);
            } catch (exception& ex) {
                log(facter::logging::level::warning, "Could not load puppet; some facts may be unavailable: %1%", ex.what());
            }
        }
        mod.search(paths);
        mod.resolve_facts();
    }

    void load_custom_facts(collection& facts, vector<string> const& paths)
    {
         load_custom_facts(facts, false, paths);
    }

    void uninitialize()
    {
        api& ruby = api::instance();
        ruby.uninitialize();
    }
}}  // namespace facter::ruby
