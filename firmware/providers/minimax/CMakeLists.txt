idf_component_register(SRCS "minimax_provider.c" "minimax_quota_mapper.c" "minimax_http.c"
                    INCLUDE_DIRS "." ".."
                    REQUIRES cjson domain http)