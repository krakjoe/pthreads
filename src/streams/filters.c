/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2018                                       |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Bastian Schneider <b.schneider@badnoob.com>                 |
  | Borrowed code from php-src                                           |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_STREAMS_FILTERS
#define HAVE_PTHREADS_STREAMS_FILTERS

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_FILTERS_H
#	include <src/streams/filters.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_STANDARD_FILTERS_H
#	include <src/streams/standard_filters.h>
#endif

int pthreads_init_stream_filters() {

	zend_declare_class_constant_long(pthreads_stream_filter_entry,  ZEND_STRL("PSFS_PASS_ON")			, PTHREADS_SFS_PASS_ON);
	zend_declare_class_constant_long(pthreads_stream_filter_entry,  ZEND_STRL("PSFS_FEED_ME")			, PTHREADS_SFS_FEED_ME);
	zend_declare_class_constant_long(pthreads_stream_filter_entry,  ZEND_STRL("PSFS_ERR_FATAL")			, PTHREADS_SFS_ERR_FATAL);

	zend_declare_class_constant_long(pthreads_stream_filter_entry,  ZEND_STRL("PSFS_FLAG_NORMAL")		, PTHREADS_SFS_FLAG_NORMAL);
	zend_declare_class_constant_long(pthreads_stream_filter_entry,  ZEND_STRL("PSFS_FLAG_FLUSH_INC")	, PTHREADS_SFS_FLAG_FLUSH_INC);
	zend_declare_class_constant_long(pthreads_stream_filter_entry,  ZEND_STRL("PSFS_FLAG_FLUSH_CLOSE")	, PTHREADS_SFS_FLAG_FLUSH_CLOSE);

	standard_filters_init();

	return SUCCESS;
}

int pthreads_shutdown_stream_filters() {

	standard_filters_shutdown();

	return SUCCESS;
}

static void pthreads_userfilter_dtor(pthreads_stream_filter_t *threaded_thisfilter) {
	pthreads_stream_filter *thisfilter = PTHREADS_FETCH_STREAMS_FILTER(threaded_thisfilter);

	zval *obj = &thisfilter->abstract;
	zval func_name;
	zval retval;

	if (obj == NULL) {
		/* If there's no object associated then there's nothing to dispose of */
		return;
	}

	ZVAL_STRINGL(&func_name, PTHREADS_STREAM_FILTER_FUNC_ONCLOSE, sizeof(PTHREADS_STREAM_FILTER_FUNC_ONCLOSE)-1);

	call_user_function(NULL,
			obj,
			&func_name,
			&retval,
			0, NULL);

	zval_ptr_dtor(&retval);
	zval_ptr_dtor(&func_name);

	/* kill the object */
	zval_ptr_dtor(obj);
}

pthreads_stream_filter_status_t pthreads_userfilter_filter(
			pthreads_stream_t *threaded_stream,
			pthreads_stream_filter_t *threaded_thisfilter,
			pthreads_stream_bucket_brigade_t *threaded_buckets_in,
			pthreads_stream_bucket_brigade_t *threaded_buckets_out,
			size_t *bytes_consumed,
			int flags
			)
{
	pthreads_stream_bucket_brigade *buckets_in, *buckets_out;
	pthreads_stream_filter_t *threaded_user_filter;
	pthreads_stream_bucket_t *threaded_bucket;
	int ret = PTHREADS_SFS_ERR_FATAL;
	zval *obj = &PTHREADS_FETCH_STREAMS_FILTER(threaded_thisfilter)->abstract;
	zval func_name, retval;
	zval args[4];
	zval zpropname;
	int call_result;

	/* the userfilter object probably doesn't exist anymore */
	if (CG(unclean_shutdown)) {
		return ret;
	}

	if(MONITOR_LOCK(threaded_thisfilter)) {
		buckets_in = PTHREADS_FETCH_STREAMS_BRIGADE(threaded_buckets_in);
		buckets_out = PTHREADS_FETCH_STREAMS_BRIGADE(threaded_buckets_out);
		threaded_user_filter = PTHREADS_FETCH_FROM(Z_OBJ_P(obj));

		if (MONITOR_LOCK(threaded_user_filter)) {
			if (!zend_hash_str_exists_ind(threaded_user_filter->store.props, PTHREADS_STREAM_FILTER_PROP_STREAM, sizeof(PTHREADS_STREAM_FILTER_PROP_STREAM)-1)) {
				zval tmp;

				/* Give the userfilter class a hook back to the stream */
				pthreads_stream_to_zval(threaded_stream, &tmp);
				Z_ADDREF(tmp);
				add_property_zval(obj, PTHREADS_STREAM_FILTER_PROP_STREAM, &tmp);
				/* add_property_zval increments the refcount which is unwanted here */
				zval_ptr_dtor(&tmp);
			}
			MONITOR_UNLOCK(threaded_user_filter);
		}

		ZVAL_STRINGL(&func_name, PTHREADS_STREAM_FILTER_FUNC_FILTER, sizeof(PTHREADS_STREAM_FILTER_FUNC_FILTER)-1);

		/* Setup calling arguments */
		ZVAL_OBJ(&args[0], PTHREADS_STD_P(threaded_buckets_in));
		ZVAL_OBJ(&args[1], PTHREADS_STD_P(threaded_buckets_out));

		if (bytes_consumed) {
			ZVAL_LONG(&args[2], *bytes_consumed);
		} else {
			ZVAL_NULL(&args[2]);
		}

		ZVAL_BOOL(&args[3], flags & PTHREADS_SFS_FLAG_FLUSH_CLOSE);

		call_result = call_user_function_ex(NULL,
				obj,
				&func_name,
				&retval,
				4, args,
				0, NULL);

		zval_ptr_dtor(&func_name);

		if (call_result == SUCCESS && Z_TYPE(retval) != IS_UNDEF) {
			convert_to_long(&retval);
			ret = (int)Z_LVAL(retval);
		} else if (call_result == FAILURE) {
			php_error_docref(NULL, E_WARNING, "failed to call filter function");
		}

		if (bytes_consumed) {
			*bytes_consumed = zval_get_long(&args[2]);
		}

		if (buckets_in->head) {
			php_error_docref(NULL, E_WARNING, "Unprocessed filter buckets remaining on input brigade");

			while ((threaded_bucket = buckets_in->head) != NULL) {
				/* Remove unconsumed buckets from the brigade */
				pthreads_stream_bucket_destroy(threaded_bucket);
			}
		}

		if (ret != PTHREADS_SFS_PASS_ON) {
			while ((threaded_bucket = buckets_out->head) != NULL) {
				pthreads_stream_bucket_destroy(threaded_bucket);
			}
		}

		/* filters are cleaned up by the stream destructor,
		 * keeping a reference to the stream here would prevent it
		 * from being destroyed properly */
		ZVAL_STRINGL(&zpropname, PTHREADS_STREAM_FILTER_PROP_STREAM, sizeof(PTHREADS_STREAM_FILTER_PROP_STREAM)-1);
		Z_OBJ_HANDLER_P(obj, unset_property)(obj, &zpropname, NULL);
		zval_ptr_dtor(&zpropname);

		MONITOR_UNLOCK(threaded_thisfilter);
	}

	zval_ptr_dtor(&args[3]);
	zval_ptr_dtor(&args[2]);

	return ret;
}

static const pthreads_stream_filter_ops pthreads_userfilter_ops = {
	pthreads_userfilter_filter,
	pthreads_userfilter_dtor,
	"user-filter"
};

static pthreads_stream_filter_t *pthreads_user_filter_factory_create(const char *filtername, zval *filterparams) {
	struct pthreads_user_filter_data *fdat = NULL;
	pthreads_stream_filter_t *threaded_filter;
	pthreads_stream_filter *filter;
	zval obj, zfilter, func_name, retval;
	zend_class_entry *filter_ce = NULL;
	size_t len;

	len = strlen(filtername);

	pthreads_hashtable *user_filter_map = &PTHREADS_STREAMG(user_filter_map);

	if(MONITOR_LOCK(user_filter_map)) {

		/* determine the classname/class entry */
		if (NULL == (fdat = zend_hash_str_find_ptr(&user_filter_map->ht, (char*)filtername, len))) {
			char *period;

			/* Userspace Filters using ambiguous wildcards could cause problems.
			   i.e.: myfilter.foo.bar will always call into myfilter.foo.*
					 never seeing myfilter.*
			   TODO: Allow failed userfilter creations to continue
					 scanning through the list */
			if ((period = strrchr(filtername, '.'))) {
				char *wildcard = safe_emalloc(len, 1, 3);

				/* Search for wildcard matches instead */
				memcpy(wildcard, filtername, len + 1); /* copy \0 */
				period = wildcard + (period - filtername);
				while (period) {
					*period = '\0';
					strncat(wildcard, ".*", 2);
					if (NULL != (fdat = zend_hash_str_find_ptr(&user_filter_map->ht, wildcard, strlen(wildcard)))) {
						period = NULL;
					} else {
						*period = '\0';
						period = strrchr(wildcard, '.');
					}
				}
				efree(wildcard);
			}
			if (fdat == NULL) {
				MONITOR_UNLOCK(user_filter_map);
				php_error_docref(NULL, E_WARNING,
						"Err, filter \"%s\" is not in the user-filter map, but somehow the user-filter-factory was invoked for it!?", filtername);
				return NULL;
			}
		}
		MONITOR_UNLOCK(user_filter_map);
	}

	/* bind the classname to the actual class */
	if (NULL == (filter_ce = zend_lookup_class(fdat->classname))) {
		php_error_docref(NULL, E_WARNING,
				"user-filter \"%s\" requires class \"%s\", but that class is not defined",
				filtername, ZSTR_VAL(fdat->classname));
		return NULL;
	}

	if(!instanceof_function(filter_ce, pthreads_threaded_entry)) {
		php_error_docref(NULL, E_WARNING,
							"user-filter \"%s\" must be an instance of Threaded",
							filtername);
		return NULL;
	}

	threaded_filter = pthreads_stream_filter_new(&pthreads_userfilter_ops, NULL);
	if (threaded_filter == NULL) {
		return NULL;
	}
	filter = PTHREADS_FETCH_STREAMS_FILTER(threaded_filter);

	/* create the user_filter object */
	object_init_ex(&obj, filter_ce);

	/* filtername */
	add_property_string(&obj, "filtername", (char*)filtername);

	/* and the parameters, if any */
	if (filterparams) {
		add_property_zval(&obj, "params", filterparams);
	} else {
		add_property_null(&obj, "params");
	}

	/* invoke the constructor */
	ZVAL_STRINGL(&func_name, PTHREADS_STREAM_FILTER_FUNC_ONCREATE, sizeof(PTHREADS_STREAM_FILTER_FUNC_ONCREATE)-1);

	call_user_function(NULL,
			&obj,
			&func_name,
			&retval,
			0, NULL);

	if (Z_TYPE(retval) != IS_UNDEF) {
		if (Z_TYPE(retval) == IS_FALSE) {
			/* User reported filter creation error "return false;" */
			zval_ptr_dtor(&retval);

			/* Kill the filter (safely) */
			ZVAL_UNDEF(&filter->abstract);

			/* Kill the object */
			zval_ptr_dtor(&obj);

			pthreads_ptr_dtor(threaded_filter);

			/* Report failure to filter_alloc */
			return NULL;
		}
		zval_ptr_dtor(&retval);
	}
	zval_ptr_dtor(&func_name);

	ZVAL_COPY_VALUE(&filter->abstract, &obj);

	/* set the filter property, this will be used during cleanup */
	ZVAL_OBJ(&zfilter, PTHREADS_STD_P(threaded_filter));
	add_property_zval(&obj, PTHREADS_STREAM_FILTER_PROP_FILTER, &zfilter);
	/* add_property_zval increments the refcount which is unwanted here */
	zval_ptr_dtor(&zfilter);

	return threaded_filter;
}

const pthreads_stream_filter_factory pthreads_user_filter_factory = {
	pthreads_user_filter_factory_create
};

pthreads_hashtable *_pthreads_get_stream_filters_hash(void) {
	return &PTHREADS_STREAMG(stream_filters_hash);
}

int pthreads_stream_filter_register_factory(const char *filterpattern, const pthreads_stream_filter_factory *factory) {
	pthreads_hashtable *stream_filters_hash = &PTHREADS_STREAMG(stream_filters_hash);
	zend_string *str = zend_string_init_interned(filterpattern, strlen(filterpattern), 1);
	int result;

	if(MONITOR_LOCK(stream_filters_hash)) {
		result = zend_hash_add_ptr(&stream_filters_hash->ht, str, (void*)factory) ? SUCCESS : FAILURE;
		MONITOR_UNLOCK(stream_filters_hash);
	}
	return result;
}

int pthreads_stream_filter_unregister_factory(const char *filterpattern) {
	pthreads_hashtable *stream_filters_hash = &PTHREADS_STREAMG(stream_filters_hash);
	int result = FAILURE;

	if(MONITOR_LOCK(stream_filters_hash)) {
		result = zend_hash_str_del(&stream_filters_hash->ht, filterpattern, strlen(filterpattern));
		MONITOR_UNLOCK(stream_filters_hash);
	}
	return result;
}

int pthreads_streams_add_user_filter_map_entry(zend_string *filtername, zend_string *classname) {
	struct pthreads_user_filter_data *fdat;

	fdat = calloc(1, sizeof(struct pthreads_user_filter_data));
	fdat->classname = classname;

	pthreads_hashtable *user_filter_map = &PTHREADS_STREAMG(user_filter_map);

	if(MONITOR_LOCK(user_filter_map)) {
		void *result = zend_hash_add_ptr(&user_filter_map->ht, filtername, fdat);

		MONITOR_UNLOCK(user_filter_map);

		if(result != NULL) {
			return SUCCESS;
		}
	}
	free(fdat);

	return FAILURE;
}

void pthreads_streams_drop_user_filter_map_entry(zend_string *filtername) {
	pthreads_hashtable *user_filter_map = &PTHREADS_STREAMG(user_filter_map);

	if(MONITOR_LOCK(user_filter_map)) {
		zend_hash_del(&user_filter_map->ht, filtername);
		MONITOR_UNLOCK(user_filter_map);
	}
}

/* We allow very simple pattern matching for filter factories:
 * if "convert.charset.utf-8/sjis" is requested, we search first for an exact
 * match. If that fails, we try "convert.charset.*", then "convert.*"
 * This means that we don't need to clog up the hashtable with a zillion
 * charsets (for example) but still be able to provide them all as filters */
pthreads_stream_filter_t *pthreads_stream_filter_create(const char *filtername, zval *filterparams) {
	pthreads_hashtable *filter_hash = pthreads_get_stream_filters_hash();
	const pthreads_stream_filter_factory *factory = NULL;
	pthreads_stream_filter_t *threaded_filter = NULL;
	size_t n;
	char *period;

	n = strlen(filtername);

	if(pthreads_monitor_lock(filter_hash->monitor)) {
		factory = zend_hash_str_find_ptr(&filter_hash->ht, filtername, n);
		pthreads_monitor_unlock(filter_hash->monitor);
	}

	if (NULL != factory) {
		threaded_filter = factory->create_filter(filtername, filterparams);
	} else if ((period = strrchr(filtername, '.'))) {
		/* try a wildcard */
		char *wildname;

		wildname = safe_emalloc(1, n, 3);
		memcpy(wildname, filtername, n+1);
		period = wildname + (period - filtername);
		while (period && !threaded_filter) {
			*period = '\0';
			strncat(wildname, ".*", 2);

			if(pthreads_monitor_lock(filter_hash->monitor)) {
				factory = zend_hash_str_find_ptr(&filter_hash->ht, wildname, strlen(wildname));
				pthreads_monitor_unlock(filter_hash->monitor);
			}

			if (NULL != factory) {
				threaded_filter = factory->create_filter(filtername, filterparams);
			}

			*period = '\0';
			period = strrchr(wildname, '.');
		}
		efree(wildname);
	}

	if (threaded_filter == NULL) {
		/* TODO: these need correct docrefs */
		if (factory == NULL)
			php_error_docref(NULL, E_WARNING, "unable to locate filter \"%s\"", filtername);
		else
			php_error_docref(NULL, E_WARNING, "unable to create or locate filter \"%s\"", filtername);
	}

	return threaded_filter;
}

pthreads_stream_filter *_pthreads_stream_filter_alloc(const pthreads_stream_filter_ops *fops, void *abstract) {
	pthreads_stream_filter *filter;

	filter = (pthreads_stream_filter*) malloc(sizeof(pthreads_stream_filter));
	memset(filter, 0, sizeof(pthreads_stream_filter));

	filter->fops = fops;
	Z_PTR(filter->abstract) = abstract;

	return filter;
}

void pthreads_stream_filter_free(pthreads_stream_filter *filter, pthreads_stream_filter_t *threaded_filter) {
	if (filter->fops->dtor)
		filter->fops->dtor(threaded_filter);
	free(filter);
}

int pthreads_stream_filter_prepend_ex(pthreads_stream_filter_chain_t *threaded_chain, pthreads_stream_filter_t *threaded_filter) {
	pthreads_stream_filter *filter = PTHREADS_FETCH_STREAMS_FILTER(threaded_filter);

	if(pthreads_streams_aquire_double_lock(threaded_filter, threaded_chain)) {
		//filter->next = chain->head;
		pthreads_filter_set_next(threaded_filter, pthreads_chain_get_head(threaded_chain));

		//filter->prev = NULL;
		pthreads_filter_set_prev(threaded_filter, NULL);

		if (pthreads_chain_has_head(threaded_chain)) {
			//PTHREADS_FETCH_STREAMS_FILTER(chain->head)->prev = threaded_filter;
			pthreads_filter_set_prev(pthreads_chain_get_head(threaded_chain), threaded_filter);
		} else {
			//chain->tail = threaded_filter;
			pthreads_chain_set_tail(threaded_chain, threaded_filter);
		}
		//chain->head = threaded_filter;
		pthreads_chain_set_head(threaded_chain, threaded_filter);
		//filter->chain = chain;
		pthreads_filter_set_chain(threaded_filter, threaded_chain);

		pthreads_add_ref(threaded_filter);

		pthreads_streams_release_double_lock(threaded_filter, threaded_chain);
	}
	return SUCCESS;
}

void _pthreads_stream_filter_prepend(pthreads_stream_filter_chain_t *threaded_chain, pthreads_stream_filter_t *threaded_filter) {
	pthreads_stream_filter_prepend_ex(threaded_chain, threaded_filter);
}

int pthreads_stream_filter_append_ex(pthreads_stream_filter_chain_t *threaded_chain, pthreads_stream_filter_t *threaded_filter) {
	pthreads_stream_t *threaded_stream = pthreads_chain_get_stream(threaded_chain);

	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_filter *filter = PTHREADS_FETCH_STREAMS_FILTER(threaded_filter);

	// reentrant lock
	if(pthreads_streams_aquire_double_lock(threaded_filter, threaded_chain)) {
		//filter->prev = chain->tail;
		pthreads_filter_set_prev(threaded_filter, pthreads_chain_get_tail(threaded_chain));

		//filter->next = NULL;
		pthreads_filter_set_next(threaded_filter, NULL);

		if (pthreads_chain_has_tail(threaded_chain)) {
			//PTHREADS_FETCH_STREAMS_FILTER(chain->tail)->next = threaded_filter;
			pthreads_filter_set_next(pthreads_chain_get_tail(threaded_chain), threaded_filter);
		} else {
			//chain->head = threaded_filter;
			pthreads_chain_set_head(threaded_chain, threaded_filter);
		}

		//chain->tail = threaded_filter;
		pthreads_chain_set_tail(threaded_chain, threaded_filter);

		//filter->chain = chain;
		pthreads_filter_set_chain(threaded_filter, threaded_chain);

		pthreads_add_ref(threaded_filter);

		pthreads_streams_release_double_lock(threaded_filter, threaded_chain);
	}

	if(stream_lock(threaded_stream)) {
		if (!pthreads_object_compare(pthreads_stream_get_readfilters(threaded_stream), threaded_chain) && (stream->writepos - stream->readpos) > 0) {
			/* Let's going ahead and wind anything in the buffer through this filter */
			pthreads_stream_bucket_brigade_t *brig_inp = pthreads_stream_bucket_brigade_new(), *brig_outp = pthreads_stream_bucket_brigade_new();
			pthreads_stream_filter_status_t status;
			pthreads_stream_bucket_t *threaded_bucket;
			pthreads_stream_bucket *bucket;
			size_t consumed = 0;

			threaded_bucket = pthreads_stream_bucket_new((char*) stream->readbuf + stream->readpos, stream->writepos - stream->readpos);
			pthreads_stream_bucket_append(brig_inp, threaded_bucket, 0);
			status = filter->fops->filter(threaded_stream, threaded_filter, brig_inp, brig_outp, &consumed, PTHREADS_SFS_FLAG_NORMAL);

			if (stream->readpos + consumed > (uint32_t)stream->writepos) {
				/* No behaving filter should cause this. */
				status = PTHREADS_SFS_ERR_FATAL;
			}
			pthreads_ptr_dtor(threaded_bucket);

			switch (status) {
				case PTHREADS_SFS_ERR_FATAL:
					while ((threaded_bucket = PTHREADS_FETCH_STREAMS_BRIGADE(brig_inp)->head) != NULL) {
						pthreads_stream_bucket_destroy(threaded_bucket);
					}

					while ((threaded_bucket = PTHREADS_FETCH_STREAMS_BRIGADE(brig_outp)->head) != NULL) {
						pthreads_stream_bucket_destroy(threaded_bucket);
					}
					php_error_docref(NULL, E_WARNING, "Filter failed to process pre-buffered data");

					pthreads_ptr_dtor(brig_inp);
					pthreads_ptr_dtor(brig_outp);

					return FAILURE;
				case PTHREADS_SFS_FEED_ME:
					/* We don't actually need data yet,
					   leave this filter in a feed me state until data is needed.
					   Reset stream's internal read buffer since the filter is "holding" it. */
					stream->readpos = 0;
					stream->writepos = 0;
					break;
				case PTHREADS_SFS_PASS_ON:
					/* If any data is consumed, we cannot rely upon the existing read buffer,
					   as the filtered data must replace the existing data, so invalidate the cache */
					/* note that changes here should be reflected in
					   src/streams/streams.c::pthreads_stream_fill_read_buffer */
					stream->writepos = 0;
					stream->readpos = 0;

					while ((threaded_bucket = PTHREADS_FETCH_STREAMS_BRIGADE(brig_outp)->head) != NULL) {
						pthreads_stream_bucket_sync_properties(threaded_bucket);
						bucket = PTHREADS_FETCH_STREAMS_BUCKET(threaded_bucket);

						/* Grow buffer to hold this bucket if need be.
						   TODO: See warning in src/streams/streams.c::pthreads_stream_fill_read_buffer */
						if (stream->readbuflen - stream->writepos < bucket->buflen) {
							stream->readbuflen += bucket->buflen;
							stream->readbuf = realloc(stream->readbuf, stream->readbuflen);
						}
						memcpy(stream->readbuf + stream->writepos, bucket->buf, bucket->buflen);
						stream->writepos += bucket->buflen;

						pthreads_stream_bucket_destroy(threaded_bucket);
					}
					break;
			}
			pthreads_ptr_dtor(brig_inp);
			pthreads_ptr_dtor(brig_outp);
		}
		pthreads_monitor_unlock(threaded_stream->monitor);
	}

	return SUCCESS;
}

void _pthreads_stream_filter_append(pthreads_stream_filter_chain_t *threaded_chain, pthreads_stream_filter_t *threaded_filter) {
	pthreads_stream_filter *filter;

	if(pthreads_streams_aquire_double_lock(threaded_filter, threaded_chain)) {
		filter = PTHREADS_FETCH_STREAMS_FILTER(threaded_filter);
		if (pthreads_stream_filter_append_ex(threaded_chain, threaded_filter) != SUCCESS) {
			if (!pthreads_object_compare(pthreads_chain_get_head(threaded_chain), threaded_filter)) {
				//chain->head = NULL;
				pthreads_chain_set_head(threaded_chain, NULL);
				//chain->tail = NULL;
				pthreads_chain_set_tail(threaded_chain, NULL);
			} else {
				//PTHREADS_FETCH_STREAMS_FILTER(filter->prev)->next = NULL;
				pthreads_filter_set_next(pthreads_filter_get_prev(threaded_filter), NULL);
				//chain->tail = filter->prev;
				pthreads_chain_set_tail(threaded_chain, pthreads_filter_get_prev(threaded_filter));
			}
		}
		pthreads_streams_release_double_lock(threaded_filter, threaded_chain);
	}
}

int _pthreads_stream_filter_flush(pthreads_stream_filter_t *threaded_filter, int finish) {
	pthreads_stream_bucket_brigade_t *inp = pthreads_stream_bucket_brigade_new(), *outp = pthreads_stream_bucket_brigade_new(), *brig_temp;
	pthreads_stream_bucket_t *threaded_bucket;
	pthreads_stream_bucket *bucket;
	pthreads_stream_filter_chain_t *threaded_chain;
	pthreads_stream_t *threaded_stream;
	pthreads_stream *stream;
	pthreads_stream_filter_t *current;
	pthreads_stream_filter *filter = PTHREADS_FETCH_STREAMS_FILTER(threaded_filter);
	size_t flushed_size = 0;
	long flags = (finish ? PTHREADS_SFS_FLAG_FLUSH_CLOSE : PTHREADS_SFS_FLAG_FLUSH_INC);

	threaded_chain = pthreads_filter_get_chain(threaded_filter);

	if (!threaded_chain) {
		/* Filter is not attached to a chain */
		return FAILURE;
	}
	threaded_stream = pthreads_chain_get_stream(threaded_chain);

	if (!threaded_stream) {
		/* Chain is somehow not part of a stream */
		return FAILURE;
	}
	if(stream_lock(threaded_stream)) {
		stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

		for(current = threaded_filter; current; current = pthreads_filter_get_next(current)) {
			pthreads_stream_filter_status_t status;

			status = filter->fops->filter(threaded_stream, current, inp, outp, NULL, flags);
			if (status == PTHREADS_SFS_FEED_ME) {
				stream_unlock(threaded_stream);
				/* We've flushed the data far enough */
				return SUCCESS;
			}
			if (status == PTHREADS_SFS_ERR_FATAL) {
				stream_unlock(threaded_stream);
				return FAILURE;
			}
			/* Otherwise we have data available to PASS_ON
				Swap the brigades and continue */
			brig_temp = inp;
			inp = outp;
			outp = brig_temp;
			PTHREADS_FETCH_STREAMS_BRIGADE(outp)->head = NULL;
			PTHREADS_FETCH_STREAMS_BRIGADE(outp)->tail = NULL;

			flags = PTHREADS_SFS_FLAG_NORMAL;
		}

		/* Last filter returned data via PTHREADS_SFS_PASS_ON
			Do something with it */

		for(threaded_bucket = PTHREADS_FETCH_STREAMS_BRIGADE(inp)->head; threaded_bucket; threaded_bucket = PTHREADS_FETCH_STREAMS_BUCKET(threaded_bucket)->next) {
			pthreads_stream_bucket_sync_properties(threaded_bucket);
			flushed_size += PTHREADS_FETCH_STREAMS_BUCKET(threaded_bucket)->buflen;
		}

		if (flushed_size == 0) {
			stream_unlock(threaded_stream);
			/* Unlikely, but possible */
			return SUCCESS;
		}

		if (!pthreads_object_compare(threaded_chain, pthreads_stream_get_readfilters(threaded_stream))) {
			/* Dump any newly flushed data to the read buffer */
			if (stream->readpos > 0) {
				/* Back the buffer up */
				memcpy(stream->readbuf, stream->readbuf + stream->readpos, stream->writepos - stream->readpos);
				stream->readpos = 0;
				stream->writepos -= stream->readpos;
			}

			if (flushed_size > (stream->readbuflen - stream->writepos)) {
				/* Grow the buffer */
				stream->readbuf = realloc(stream->readbuf, stream->writepos + flushed_size + stream->chunk_size);
			}

			while ((threaded_bucket = PTHREADS_FETCH_STREAMS_BRIGADE(inp)->head)) {
				bucket = PTHREADS_FETCH_STREAMS_BUCKET(threaded_bucket);
				memcpy(stream->readbuf + stream->writepos, bucket->buf, bucket->buflen);
				stream->writepos += bucket->buflen;
				pthreads_stream_bucket_destroy(threaded_bucket);
			}
		} else if (!pthreads_object_compare(threaded_chain, pthreads_stream_get_writefilters(threaded_stream))) {
			/* Send flushed data to the stream */
			while ((threaded_bucket = PTHREADS_FETCH_STREAMS_BRIGADE(inp)->head)) {
				bucket = PTHREADS_FETCH_STREAMS_BUCKET(threaded_bucket);
				stream->ops->write(threaded_stream, bucket->buf, bucket->buflen);
				pthreads_stream_bucket_destroy(threaded_bucket);
			}
		}
		pthreads_ptr_dtor(inp);
		pthreads_ptr_dtor(outp);

		stream_unlock(threaded_stream);
	}

	return SUCCESS;
}

int _pthreads_stream_filter_is_integrated(pthreads_stream_filter_t *threaded_filter) {
	return pthreads_filter_has_chain(threaded_filter);
}

pthreads_stream_filter_t *_pthreads_stream_filter_remove(pthreads_stream_filter_t *threaded_filter, int call_dtor) {
	pthreads_stream_filter *filter = PTHREADS_FETCH_STREAMS_FILTER(threaded_filter);
	pthreads_stream_filter_t *next, *prev;

	if(MONITOR_LOCK(threaded_filter)) {
		prev = pthreads_filter_get_prev(threaded_filter);
		next = pthreads_filter_get_next(threaded_filter);

		if (prev) {
			//PTHREADS_FETCH_STREAMS_FILTER(prev)->next = next;
			pthreads_filter_set_next(prev, next);
		} else {
			//filter->chain->head = next;
			pthreads_chain_set_head(pthreads_filter_get_chain(threaded_filter), next);
		}

		if (next) {
			//PTHREADS_FETCH_STREAMS_FILTER(next)->prev = prev;
			pthreads_filter_set_prev(next, prev);
		} else {
			//filter->chain->tail = prev;
			pthreads_chain_set_tail(pthreads_filter_get_chain(threaded_filter), prev);
		}
		//pthreads_filter_set_chain(threaded_filter, NULL);

		MONITOR_UNLOCK(threaded_filter);
	}

	if (call_dtor) {
		pthreads_ptr_dtor(threaded_filter);
		return NULL;
	}
	return threaded_filter;
}

#ifndef HAVE_PTHREADS_API_FILTERS
#	include "src/streams/filters_api.c"
#endif

#endif
