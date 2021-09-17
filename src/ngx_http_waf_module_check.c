#include <ngx_http_waf_module_check.h>

extern ngx_module_t ngx_http_waf_module; /**< 模块详情 */

ngx_int_t ngx_http_waf_handler_check_white_ip(ngx_http_request_t* r, ngx_int_t* out_http_status) {
    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: Start inspecting the IP whitelist.");

    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);

    ngx_int_t ret_value = NGX_HTTP_WAF_NOT_MATCHED;

    if (ngx_http_waf_check_flag(loc_conf->waf_mode, NGX_HTTP_WAF_MODE_INSPECT_IP) == NGX_HTTP_WAF_FALSE) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Because this Inspection is disabled in the configuration, no Inspection is performed.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection has begun.");

        ip_trie_node_t* ip_trie_node = NULL;
        if (r->connection->sockaddr->sa_family == AF_INET) {
            struct sockaddr_in* sin = (struct sockaddr_in*)r->connection->sockaddr;
            inx_addr_t inx_addr;
            ngx_memcpy(&(inx_addr.ipv4), &(sin->sin_addr), sizeof(struct in_addr));
            if (ip_trie_find(loc_conf->white_ipv4, &inx_addr, &ip_trie_node) == NGX_HTTP_WAF_SUCCESS) {
                ctx->blocked = NGX_HTTP_WAF_FALSE;
                strcpy((char*)ctx->rule_type, "WHITE-IPV4");
                strcpy((char*)ctx->rule_deatils, (char*)ip_trie_node->data);
                *out_http_status = NGX_DECLINED;
                ret_value = NGX_HTTP_WAF_MATCHED;
            }
        } 
#if (NGX_HAVE_INET6)
        else if (r->connection->sockaddr->sa_family == AF_INET6) {
            struct sockaddr_in6* sin6 = (struct sockaddr_in6*)r->connection->sockaddr;
            inx_addr_t inx_addr;
            
            ngx_memcpy(&(inx_addr.ipv6), &(sin6->sin6_addr), sizeof(struct in6_addr));
            if (ip_trie_find(loc_conf->white_ipv6, &inx_addr, &ip_trie_node) == NGX_HTTP_WAF_SUCCESS) {
                ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
                ctx->blocked = NGX_HTTP_WAF_FALSE;
                strcpy((char*)ctx->rule_type, "WHITE-IPV6");
                strcpy((char*)ctx->rule_deatils, (char*)ip_trie_node->data);
                *out_http_status = NGX_DECLINED;
                ret_value = NGX_HTTP_WAF_MATCHED;
            }
        }
#endif

        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection is over.");
    }

    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: The IP whitelist inspection process is fully completed.");
    return ret_value;
}


ngx_int_t ngx_http_waf_handler_check_black_ip(ngx_http_request_t* r, ngx_int_t* out_http_status) {
    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: Start inspecting the IP blacklist.");

    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);
    
    ngx_int_t ret_value = NGX_HTTP_WAF_NOT_MATCHED;

    if (ngx_http_waf_check_flag(loc_conf->waf_mode, NGX_HTTP_WAF_MODE_INSPECT_IP) == NGX_HTTP_WAF_FALSE) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Because this Inspection is disabled in the configuration, no Inspection is performed.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection has begun.");

        ip_trie_node_t *ip_trie_node = NULL;
        if (r->connection->sockaddr->sa_family == AF_INET) {
            struct sockaddr_in* sin = (struct sockaddr_in*)r->connection->sockaddr;
            inx_addr_t inx_addr;
            
            ngx_memcpy(&(inx_addr.ipv4), &(sin->sin_addr), sizeof(struct in_addr));
            if (ip_trie_find(loc_conf->black_ipv4, &inx_addr, &ip_trie_node) == NGX_HTTP_WAF_SUCCESS) {
                ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
                ctx->blocked = NGX_HTTP_WAF_TRUE;
                strcpy((char*)ctx->rule_type, "BLACK-IPV4");
                strcpy((char*)ctx->rule_deatils, (char*)ip_trie_node->data);
                *out_http_status = NGX_HTTP_FORBIDDEN;
                ret_value = NGX_HTTP_WAF_MATCHED;
            }
        } 
#if (NGX_HAVE_INET6)
        else if (r->connection->sockaddr->sa_family == AF_INET6) {
            struct sockaddr_in6* sin6 = (struct sockaddr_in6*)r->connection->sockaddr;
            inx_addr_t inx_addr;
            ngx_memcpy(&(inx_addr.ipv6), &(sin6->sin6_addr), sizeof(struct in6_addr));
            if (ip_trie_find(loc_conf->black_ipv6, &inx_addr, &ip_trie_node) == NGX_HTTP_WAF_SUCCESS) {
                ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
                ctx->blocked = NGX_HTTP_WAF_TRUE;
                strcpy((char*)ctx->rule_type, "BLACK-IPV6");
                strcpy((char*)ctx->rule_deatils, (char*)ip_trie_node->data);
                *out_http_status = loc_conf->waf_http_status;
                ret_value = NGX_HTTP_WAF_MATCHED;
            }
        }
#endif

        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection is over.");
    }

    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: The IP blacklist inspection process is fully completed.");
    return ret_value;
}


ngx_int_t ngx_http_waf_handler_check_cc(ngx_http_request_t* r, ngx_int_t* out_http_status) {
    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: Start the CC inspection process.");

    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);
    
    ngx_int_t ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    ngx_int_t ip_type = r->connection->sockaddr->sa_family;
    time_t now = time(NULL);
    
    if (loc_conf->waf_cc_deny == 0 || loc_conf->waf_cc_deny == NGX_CONF_UNSET) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Because this detection is disabled in the configuration, no detection is performed.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else if (loc_conf->waf_cc_deny_limit == NGX_CONF_UNSET
        || loc_conf->waf_cc_deny_duration == NGX_CONF_UNSET) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Because this detection is disabled in the configuration, no detection is performed.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Detection has begun.");

        inx_addr_t inx_addr;
        ngx_memset(&inx_addr, 0, sizeof(inx_addr_t));

        if (ip_type == AF_INET) {
            struct sockaddr_in* s_addr_in = (struct sockaddr_in*)(r->connection->sockaddr);
            ngx_memcpy(&(inx_addr.ipv4), &(s_addr_in->sin_addr), sizeof(struct in_addr));
        } 
#if (NGX_HAVE_INET6)
        else {
            struct sockaddr_in6* s_addr_in6 = (struct sockaddr_in6*)(r->connection->sockaddr);
            ngx_memcpy(&(inx_addr.ipv6), &(s_addr_in6->sin6_addr), sizeof(struct in6_addr));
        }
#endif
        ngx_int_t limit  = loc_conf->waf_cc_deny_limit;
        ngx_int_t duration = loc_conf->waf_cc_deny_duration;
        ip_statis_t* statis = NULL;
        ngx_slab_pool_t *shpool = (ngx_slab_pool_t *)loc_conf->shm_zone_cc_deny->shm.addr;

        ngx_shmtx_lock(&shpool->mutex);
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Shared memory is locked.");

        // randombytes_buf(&inx_addr, sizeof(inx_addr_t));

        lru_cache_find_result_t tmp0 = lru_cache_find(loc_conf->ip_access_statistics, &inx_addr, sizeof(inx_addr_t));
        if (tmp0.status == NGX_HTTP_WAF_KEY_EXISTS) {
            statis = *(tmp0).data;
        } else {
            lru_cache_add_result_t tmp1 = lru_cache_add(loc_conf->ip_access_statistics, &inx_addr, sizeof(inx_addr_t));
            if (tmp1.status == NGX_HTTP_WAF_SUCCESS) {
                statis = mem_pool_calloc(&loc_conf->ip_access_statistics->pool, sizeof(ip_statis_t));
                assert(statis != NULL);
                statis->count = 1;
                statis->is_blocked = NGX_HTTP_WAF_FALSE;
                statis->record_time = now;
                statis->block_time = 0;
                statis->bad_captcha_count = 0;

                *(tmp1.data) = statis;
            } else {
                goto exception;
            }
        }

        double diff_second_record = difftime(now, statis->record_time);
        double diff_second_block = difftime(now, statis->block_time);

        /* 如果已经被拦截 */
        if (statis->is_blocked == NGX_HTTP_WAF_TRUE) {
            /* 如果还在拦截时间内 */
            if (diff_second_block < duration) {
                goto matched;
            } else {
                statis->count = 1;
                statis->is_blocked = NGX_HTTP_WAF_FALSE;
                statis->record_time = now;
                statis->block_time = 0;
                statis->bad_captcha_count = 0;
            }
        }
        /* 如果还在一个统计周期内 */ 
        else if (diff_second_record <= loc_conf->waf_cc_deny_cycle) {
            /* 如果访问频率超出限制 */
            if (statis->count > limit) {
                goto matched;
            } else {
                ++(statis->count);
            }
        } else {
            statis->count = 1;
            statis->is_blocked = NGX_HTTP_WAF_FALSE;
            statis->record_time = now;
            statis->block_time = 0;
            statis->bad_captcha_count = 0;
        }


        goto unlock;

        matched: 
        {
            if (loc_conf->waf_cc_deny == 2) {
                goto captcha;
            } else {
                goto block;
            }
        }

        captcha:
        {
            if (statis->bad_captcha_count >= 3) {
                goto block;
            }
            ngx_int_t temp = (ngx_int_t)ngx_max(loc_conf->waf_cc_deny_limit * 1.2, loc_conf->waf_cc_deny_limit + 20);
            if (statis->count > temp) {
                goto block;
            }
            switch (ngx_http_waf_captcha_test(r, out_http_status)) {
                case NGX_HTTP_WAF_BAD:
                    goto exception;
                case NGX_HTTP_WAF_CAPTCHA_PASS:
                    statis->is_blocked = NGX_HTTP_WAF_FALSE;
                    statis->count = 0;
                    statis->record_time = now;
                    statis->block_time = 0;
                    statis->bad_captcha_count = 0;
                    ret_value = NGX_HTTP_WAF_NOT_MATCHED;
                    break;
                case NGX_HTTP_WAF_CAPTCHA_BAD:
                    ++(statis->bad_captcha_count);
                    ret_value = NGX_HTTP_WAF_NOT_MATCHED;
                    break;
                case NGX_HTTP_WAF_CAPTCHA_CHALLENGE:
                    ++(statis->count);
                    *out_http_status = NGX_DECLINED;
                    ret_value = NGX_HTTP_WAF_MATCHED;
                    break;
            }
            goto unlock;
        }

        block: 
        {
            if (statis->is_blocked == NGX_HTTP_WAF_FALSE) {
                statis->is_blocked = NGX_HTTP_WAF_TRUE;
                statis->block_time = now;
            }

            ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
            ctx->blocked = NGX_HTTP_WAF_TRUE;
            strcpy((char*)ctx->rule_type, "CC-DENY");
            strcpy((char*)ctx->rule_deatils, "");
            *out_http_status = loc_conf->waf_http_status_cc;
            ret_value = NGX_HTTP_WAF_MATCHED;
            time_t remain = duration - (now - statis->block_time);

            if (loc_conf->waf_http_status_cc != NGX_HTTP_CLOSE) {
                ngx_table_elt_t* header = (ngx_table_elt_t*)ngx_list_push(&(r->headers_out.headers));
                if (header == NULL) {
                    goto unlock;
                }

                /* 如果 hash 字段为 0 则会在遍历 HTTP 头的时候被忽略 */
                header->hash = 1;
                header->lowcase_key = (u_char*)"Retry-After";
                ngx_str_set(&header->key, "Retry-After");
                header->value.data = ngx_palloc(r->pool, NGX_TIME_T_LEN + 1);
                if (header->value.data == NULL) {
                    goto unlock;
                }

                #if (NGX_TIME_T_SIZE == 4)
                    header->value.len = sprintf((char*)header->value.data, "%d", (int)remain);
                #elif (NGX_TIME_T_SIZE == 8)
                    header->value.len = sprintf((char*)header->value.data, "%lld", (long long)remain);
                #else
                    #error The size of time_t is unexpected.
                #endif
            }

            
            goto unlock;
        }
        
        exception:
        {
            *out_http_status = NGX_HTTP_INTERNAL_SERVER_ERROR;
            ret_value = NGX_HTTP_WAF_MATCHED;
            goto unlock;
        }
        // no_memory:
        // not_matched:
        unlock:
        
        ngx_shmtx_unlock(&shpool->mutex);
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Shared memory is unlocked.");

        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Detection is over.");

    }

    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: The CC detection process is fully completed.");
    return ret_value;
}


ngx_int_t ngx_http_waf_handler_check_white_url(ngx_http_request_t* r, ngx_int_t* out_http_status) {
    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: Start inspecting the URL whitelist.");

    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);

    ngx_int_t ret_value = NGX_HTTP_WAF_NOT_MATCHED;

    if (ngx_http_waf_check_flag(loc_conf->waf_mode, NGX_HTTP_WAF_MODE_INSPECT_URL | r->method) == NGX_HTTP_WAF_FALSE) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Because this detection is disabled in the configuration, no detection is performed.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection has begun.");

        ngx_str_t* p_uri = &r->uri;
        ngx_array_t* regex_array = loc_conf->white_url;
        lru_cache_t* cache = loc_conf->white_url_inspection_cache;

        ret_value = ngx_http_waf_regex_exec_arrray(r, p_uri, regex_array, (u_char*)"WHITE-URL", cache);

        if (ret_value == NGX_HTTP_WAF_MATCHED) {
            ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
            ctx->blocked = NGX_HTTP_WAF_FALSE;
            *out_http_status = NGX_DECLINED;
        }

        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection is over.");
    }

    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: The URL whitelist inspection process is fully completed.");
    return ret_value;
}


ngx_int_t ngx_http_waf_handler_check_black_url(ngx_http_request_t* r, ngx_int_t* out_http_status) {
    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: Start inspecting the URL blacklist.");

    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);

    ngx_int_t ret_value = NGX_HTTP_WAF_NOT_MATCHED;

    if (ngx_http_waf_check_flag(loc_conf->waf_mode, NGX_HTTP_WAF_MODE_INSPECT_URL | r->method) == NGX_HTTP_WAF_FALSE) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Because this Inspection is disabled in the configuration, no Inspection is performed.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection has begun.");

        ngx_str_t* p_uri = &r->uri;
        ngx_array_t* regex_array = loc_conf->black_url;
        lru_cache_t* cache = loc_conf->black_url_inspection_cache;

        ret_value = ngx_http_waf_regex_exec_arrray(r, p_uri, regex_array, (u_char*)"BLACK-URL", cache);

        if (ret_value == NGX_HTTP_WAF_MATCHED) {
            ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
            ctx->blocked = NGX_HTTP_WAF_TRUE;
            *out_http_status = loc_conf->waf_http_status;
        }

        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection is over.");
    }

    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: The URL blacklist inspection process is fully completed.");
    return ret_value;
}


ngx_int_t ngx_http_waf_handler_check_black_args(ngx_http_request_t* r, ngx_int_t* out_http_status) {
    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: Start inspecting the ARGS blacklist.");

    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);

    ngx_int_t ret_value = NGX_HTTP_WAF_NOT_MATCHED;

    if (ngx_http_waf_check_flag(loc_conf->waf_mode, NGX_HTTP_WAF_MODE_INSPECT_ARGS | r->method) == NGX_HTTP_WAF_FALSE) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Because this Inspection is disabled in the configuration, no Inspection is performed.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection has begun.");

        ngx_str_t* p_args = &r->args;
        ngx_array_t* regex_array = loc_conf->black_args;
        lru_cache_t* cache = loc_conf->black_args_inspection_cache;

        ret_value = ngx_http_waf_regex_exec_arrray(r, p_args, regex_array, (u_char*)"BLACK-ARGS", cache);

        if (ret_value != NGX_HTTP_WAF_MATCHED) {
            UT_array* args = NULL;
            ngx_http_waf_str_split(p_args, '&', p_args->len, &args);
            ngx_str_t* p = NULL;
            while (p = (ngx_str_t*)utarray_next(args, p), p != NULL) {
                UT_array* key_value = NULL;
                if (ngx_http_waf_str_split(p, '=', p_args->len, &key_value) == NGX_HTTP_WAF_TRUE
                    && utarray_len(key_value) == 2) {
                    ngx_str_t* key = NULL;
                    ngx_str_t* value = NULL;
                    key = (ngx_str_t*)utarray_next(key_value, NULL);
                    value = (ngx_str_t*)utarray_next(key_value, key);
                    ret_value = ngx_http_waf_regex_exec_arrray(r, key, regex_array, (u_char*)"BLACK-ARGS", cache);
                    if (ret_value == NGX_HTTP_WAF_MATCHED) {
                        break;
                    }

                    ret_value = ngx_http_waf_regex_exec_arrray(r, value, regex_array, (u_char*)"BLACK-ARGS", cache);

                    if (ret_value == NGX_HTTP_WAF_MATCHED) {
                        break;
                    }
                }
                utarray_free(key_value);
            }
            utarray_free(args);
        }

        if (ret_value == NGX_HTTP_WAF_MATCHED) {
            ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
            ctx->blocked = NGX_HTTP_WAF_TRUE;
            *out_http_status = loc_conf->waf_http_status;
        }

        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection is over.");
    }

    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: The ARGS blacklist inspection process is fully completed.");
    return ret_value;
}


ngx_int_t ngx_http_waf_handler_check_black_user_agent(ngx_http_request_t* r, ngx_int_t* out_http_status) {
    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: Start inspecting the User-Agent blacklist.");

    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);

    ngx_int_t ret_value = NGX_HTTP_WAF_NOT_MATCHED;

    if (ngx_http_waf_check_flag(loc_conf->waf_mode, NGX_HTTP_WAF_MODE_INSPECT_UA | r->method) == NGX_HTTP_WAF_FALSE) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Because this Inspection is disabled in the configuration, no Inspection is performed.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else if (r->headers_in.user_agent == NULL) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: The Inspection is skipped because the User-Agent is empty.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection has begun.");

        ngx_str_t* p_ua = &r->headers_in.user_agent->value;
        ngx_array_t* regex_array = loc_conf->black_ua;
        lru_cache_t* cache = loc_conf->black_ua_inspection_cache;

        ret_value = ngx_http_waf_regex_exec_arrray(r, p_ua, regex_array, (u_char*)"BLACK-UA", cache);

        if (ret_value == NGX_HTTP_WAF_MATCHED) {
            ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
            ctx->blocked = NGX_HTTP_WAF_TRUE;
            *out_http_status = loc_conf->waf_http_status;
        }

        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection is over.");
    }

    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: The User-Agent blacklist inspection process is fully completed.");
    return ret_value;
}


ngx_int_t ngx_http_waf_handler_check_white_referer(ngx_http_request_t* r, ngx_int_t* out_http_status) {
    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: Start inspecting the Referer whitelist.");

    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);

    ngx_int_t ret_value = NGX_HTTP_WAF_NOT_MATCHED;

    if (ngx_http_waf_check_flag(loc_conf->waf_mode, NGX_HTTP_WAF_MODE_INSPECT_REFERER | r->method) == NGX_HTTP_WAF_FALSE) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Because this Inspection is disabled in the configuration, no Inspection is performed.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else if (r->headers_in.referer == NULL) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: The Inspection is skipped because the Referer is empty.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection has begun.");

        ngx_str_t* p_referer = &r->headers_in.referer->value;
        ngx_array_t* regex_array = loc_conf->white_referer;
        lru_cache_t* cache = loc_conf->white_referer_inspection_cache;

        ret_value = ngx_http_waf_regex_exec_arrray(r, p_referer, regex_array, (u_char*)"WHITE-REFERER", cache);

        if (ret_value == NGX_HTTP_WAF_MATCHED) {
            ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
            ctx->blocked = NGX_HTTP_WAF_FALSE;
            *out_http_status = NGX_DECLINED;
        }


        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection is over.");
    }

    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: The Referer whitelist inspection process is fully completed.");
    return ret_value;
}


ngx_int_t ngx_http_waf_handler_check_black_referer(ngx_http_request_t* r, ngx_int_t* out_http_status) {
    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: Start inspecting the Referer blacklist.");

    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);

    ngx_int_t ret_value = NGX_HTTP_WAF_NOT_MATCHED;

    if (ngx_http_waf_check_flag(loc_conf->waf_mode, NGX_HTTP_WAF_MODE_INSPECT_REFERER | r->method) == NGX_HTTP_WAF_FALSE) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Because this Inspection is disabled in the configuration, no Inspection is performed.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else if (r->headers_in.referer == NULL) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: The Inspection is skipped because the Referer is empty.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection has begun.");

        ngx_str_t* p_referer = &r->headers_in.referer->value;
        ngx_array_t* regex_array = loc_conf->black_referer;
        lru_cache_t* cache = loc_conf->black_referer_inspection_cache;

        ret_value = ngx_http_waf_regex_exec_arrray(r, p_referer, regex_array, (u_char*)"BLACK-REFERER", cache);

        if (ret_value == NGX_HTTP_WAF_MATCHED) {
            ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
            ctx->blocked = NGX_HTTP_WAF_TRUE;
            *out_http_status = loc_conf->waf_http_status;
        }


        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection is over.");
    }

    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: The Referer blacklist inspection process is fully completed.");
    return ret_value;
}


ngx_int_t ngx_http_waf_handler_check_black_cookie(ngx_http_request_t* r, ngx_int_t* out_http_status) {
    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: Start inspecting the Cookie blacklist.");

    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);

    ngx_int_t ret_value = NGX_HTTP_WAF_NOT_MATCHED;

    if (ngx_http_waf_check_flag(loc_conf->waf_mode, NGX_HTTP_WAF_MODE_INSPECT_COOKIE | r->method) == NGX_HTTP_WAF_FALSE) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Because this Inspection is disabled in the configuration, no Inspection is performed.");
        ret_value = NGX_HTTP_WAF_NOT_MATCHED;
    } else if (r->headers_in.cookies.nelts != 0) {
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection has begun.");

        ngx_table_elt_t** ppcookie = r->headers_in.cookies.elts;
        size_t i;
        for (i = 0; i < r->headers_in.cookies.nelts; i++, ppcookie++) {
            ngx_str_t* native_cookies = &((**ppcookie).value);
            UT_array* cookies = NULL;
            if (ngx_http_waf_parse_cookie(native_cookies, &cookies) != NGX_HTTP_WAF_SUCCESS) {
                continue;
            }

            ngx_str_t* key = NULL;
            ngx_str_t* value = NULL;
            ngx_str_t* p = NULL;

            do {
                if (key = (ngx_str_t*)utarray_next(cookies, p), p = key, key == NULL) {
                    break;
                }

                if (value = (ngx_str_t*)utarray_next(cookies, p), p = value, value == NULL) {
                    break;
                }

                ngx_str_t temp;
                temp.len = key->len + value->len;
                temp.data = (u_char*)ngx_pcalloc(r->pool, sizeof(u_char*) * temp.len);
                ngx_memcpy(temp.data, key->data, key->len);
                ngx_memcpy(temp.data + key->len, value->data, sizeof(u_char) * value->len);

                ngx_array_t* regex_array = loc_conf->black_cookie;
                lru_cache_t* cache = loc_conf->black_cookie_inspection_cache;

                ret_value = ngx_http_waf_regex_exec_arrray(r, &temp, regex_array, (u_char*)"BLACK-COOKIE", cache);

                if (ret_value != NGX_HTTP_WAF_MATCHED) {
                    ret_value = ngx_http_waf_regex_exec_arrray(r, key, regex_array, (u_char*)"BLACK-COOKIE", cache);
                }

                if (ret_value != NGX_HTTP_WAF_MATCHED) {
                    ret_value = ngx_http_waf_regex_exec_arrray(r, value, regex_array, (u_char*)"BLACK-COOKIE", cache);
                }

                ngx_pfree(r->pool, temp.data);
                
                if (ret_value == NGX_HTTP_WAF_MATCHED) {
                    ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
                    ctx->blocked = NGX_HTTP_WAF_TRUE;
                    *out_http_status = loc_conf->waf_http_status;
                    break;
                }

            } while (p != NULL);

            utarray_free(cookies);

            if (ctx->blocked == NGX_HTTP_WAF_TRUE) {
                break;
            }
        }

        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection is over.");
    }

    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: The Cookie blacklist inspection process is fully completed.");
    return ret_value;
}


ngx_int_t ngx_http_waf_handler_check_black_post(ngx_http_request_t* r, ngx_int_t* out_http_status) {
    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: Start inspecting the Post-Body blacklist.");

    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);

    if (ngx_http_waf_check_flag(r->method, NGX_HTTP_POST) != NGX_HTTP_WAF_TRUE
    ||  ngx_http_waf_check_flag(loc_conf->waf_mode, NGX_HTTP_WAF_MODE_INSPECT_RB) != NGX_HTTP_WAF_TRUE) {
        return NGX_HTTP_WAF_NOT_MATCHED;
    }

    if (ctx->has_req_body == NGX_HTTP_WAF_FALSE) {
        return NGX_HTTP_WAF_NOT_MATCHED;
    }

    ngx_str_t body_str;
    body_str.data = ctx->req_body.pos;
    body_str.len = ctx->req_body.last - ctx->req_body.pos;

    if (ngx_http_waf_regex_exec_arrray(r, &body_str, loc_conf->black_post, (u_char*)"BLACK-POST", NULL) == NGX_HTTP_WAF_MATCHED) {
        ctx->gernal_logged = NGX_HTTP_WAF_TRUE;
        ctx->blocked = NGX_HTTP_WAF_TRUE;
    }

    ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: Inspection is over.");

    if (ctx->blocked != NGX_HTTP_WAF_TRUE) {
        return NGX_HTTP_WAF_NOT_MATCHED;
    } else {
        *out_http_status = loc_conf->waf_http_status;
        return NGX_HTTP_WAF_MATCHED;
    }
}


void ngx_http_waf_get_ctx_and_conf(ngx_http_request_t* r, ngx_http_waf_loc_conf_t** conf, ngx_http_waf_ctx_t** ctx) {
    if (ctx != NULL) {
        *ctx = NULL;
        *ctx = ngx_http_get_module_ctx(r, ngx_http_waf_module);
        if (*ctx == NULL) {
            ngx_http_cleanup_t* cln = NULL;
            for (cln = r->cleanup; cln != NULL; cln = cln->next) {
                if (cln->handler == ngx_http_waf_handler_cleanup) {
                    *ctx = cln->data;
                }
            }
        }
        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
            "ngx_waf_debug: The module context has been obtained.");
    }
    
    if (conf != NULL) {
        *conf = ngx_http_get_module_loc_conf(r, ngx_http_waf_module);
        ngx_http_waf_loc_conf_t* parent = (*conf)->parent;
        while ((*conf)->waf_cc_deny_limit == NGX_CONF_UNSET && parent != NULL) {
            (*conf)->waf_cc_deny = parent->waf_cc_deny;
            (*conf)->waf_cc_deny_limit = parent->waf_cc_deny_limit;
            (*conf)->waf_cc_deny_duration = parent->waf_cc_deny_duration;
            (*conf)->waf_cc_deny_cycle = parent->waf_cc_deny_cycle;
            (*conf)->waf_cc_deny_shm_zone_size = parent->waf_cc_deny_shm_zone_size;
            (*conf)->shm_zone_cc_deny = parent->shm_zone_cc_deny;
            (*conf)->ip_access_statistics = parent->ip_access_statistics;
            parent = parent->parent;
        }

        ngx_log_debug(NGX_LOG_DEBUG_CORE, r->connection->log, 0, 
        "ngx_waf_debug: The configuration of the module has been obtained.");
    }
}


ngx_int_t ngx_http_waf_regex_exec_arrray(ngx_http_request_t* r, 
                                         ngx_str_t* str, 
                                         ngx_array_t* array, 
                                         const u_char* rule_type, 
                                         lru_cache_t* cache) {
    ngx_http_waf_loc_conf_t* loc_conf = NULL;
    ngx_http_waf_ctx_t* ctx = NULL;
    ngx_http_waf_get_ctx_and_conf(r, &loc_conf, &ctx);
    ngx_int_t cache_hit = NGX_HTTP_WAF_FAIL;
    check_result_t result;
    result.is_matched = NGX_HTTP_WAF_NOT_MATCHED;
    result.detail = NULL;

    if (str == NULL || str->data == NULL || str->len == 0 || array == NULL || array == NGX_CONF_UNSET_PTR) {
        return NGX_HTTP_WAF_NOT_MATCHED;
    }

    if (   loc_conf->waf_cache == 1
        && loc_conf->waf_cache_capacity != NGX_CONF_UNSET
        && cache != NULL) {
        lru_cache_find_result_t tmp = lru_cache_find(cache, str->data, sizeof(u_char) * str->len);
        if (tmp.status == NGX_HTTP_WAF_KEY_EXISTS) {
            cache_hit = NGX_HTTP_WAF_SUCCESS;
            ngx_memcpy(&result, *(tmp.data), sizeof(check_result_t));
        }
    }

    if (cache_hit != NGX_HTTP_WAF_SUCCESS) {
        ngx_regex_elt_t* p = (ngx_regex_elt_t*)(array->elts);
        for (size_t i = 0; i < array->nelts; i++, p++) {
            ngx_int_t rc = ngx_regex_exec(p->regex, str, NULL, 0);
            if (rc >= 0) {
                result.is_matched = NGX_HTTP_WAF_MATCHED;
                result.detail = p->name;
                break;
            }
        }
    }

    if (   loc_conf->waf_cache == 1
        && loc_conf->waf_cache_capacity != NGX_CONF_UNSET
        && cache != NULL) {
        // lru_cache_manager_add(cache, str->data, str->len * sizeof(u_char), is_matched, rule_detail);
        lru_cache_add_result_t tmp = lru_cache_add(cache, str->data, str->len * sizeof(u_char));
        if (tmp.status == NGX_HTTP_WAF_SUCCESS) {
            *(tmp.data) = lru_cache_calloc(cache, sizeof(check_result_t));
            ngx_memcpy(*(tmp.data), &result, sizeof(check_result_t));
        }
    }

    if (result.is_matched == NGX_HTTP_WAF_MATCHED) {
        ngx_strcpy(ctx->rule_type, rule_type);
        ngx_strcpy(ctx->rule_deatils, result.detail);
    }

    return result.is_matched;
}
