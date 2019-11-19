#ifndef __CACHE_H__
#define __CACHE_H__

#ifdef __cplusplus
extern "C" {
#endif

	//������cache��
	struct cacheMan* cacheCreator(int capacity, int elem_t);

	//������cache��
	void cacheDestroy(struct cacheMan* cache, int elem_t);

	//�ڳ��з���һ��cache��ȥ
	void* cacheAllocer(struct cacheMan* cache, int elem_t);

	//�ͷų��е�cache
	void cacheFree(struct cacheMan* cache, void* cacheMem);

	void* smallCacheAllocer(int elem_t);

	void smallCacheFree(void* mem);

	char* strdupCache(char* str);

#ifdef __cplusplus
}
#endif 


#endif

