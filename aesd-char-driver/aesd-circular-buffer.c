/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif
#include <stdio.h>

#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
   struct aesd_buffer_entry* theEntryToReturn = NULL;   /* This is the entry to return, if found. */
   unsigned int              aux_counter      = 0;
   
   unsigned int              upperLimit       = 0;
   
   if (buffer->out_offs < buffer->in_offs) {
      /**/
      upperLimit = buffer->in_offs;
   }
   else {
      /* buffer->out_offs >= buffer->in_offs */
      upperLimit = AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
   }
    aux_counter = buffer->out_offs;
    while ((char_offset > (buffer->entry[aux_counter].size-1)) &&
           (aux_counter < upperLimit)) {
      char_offset -= buffer->entry[aux_counter].size;
      aux_counter++;
    }
    
    /* char_offset <= buffer->entry[aux_counter]->size || aux_counter == buffer->in_offs */
    if (aux_counter == upperLimit) {
       /* Entry not found */
       if (buffer->in_offs <= buffer->out_offs) {
          aux_counter = 0;
          while ((char_offset > (buffer->entry[aux_counter].size-1)) &&
                 (aux_counter < buffer->in_offs)) {
             char_offset -= buffer->entry[aux_counter].size;
             aux_counter++;
          }
          
          if (aux_counter < buffer->in_offs) {
             theEntryToReturn = &buffer->entry[aux_counter];
             *entry_offset_byte_rtn = char_offset;
          }
       }
    }
    else {
       /* found the entry containing the char_offset */
       theEntryToReturn = &buffer->entry[aux_counter];
       *entry_offset_byte_rtn = char_offset;
    }
    
    return theEntryToReturn;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
   /* First, checking if the buffer is not null */
   if (!buffer->full) {
      buffer->entry[buffer->in_offs].buffptr = add_entry->buffptr;
      buffer->entry[buffer->in_offs].size    = add_entry->size;
      buffer->in_offs++;

      buffer->in_offs = (buffer->in_offs % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED);
      
      if (buffer->in_offs == buffer->out_offs) {
         buffer->full = true;
      }
   }
   else {
      /* The buffer is full. Removing the first element in the buffer. */
      /* buffer->out_offs == buffer->in_offs*/
      buffer->out_offs++;
      buffer->out_offs = (buffer->out_offs % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED);
      buffer->entry[buffer->in_offs].buffptr = add_entry->buffptr;
      buffer->entry[buffer->in_offs].size    = add_entry->size;
      buffer->in_offs++;
      buffer->in_offs = (buffer->in_offs % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED);
   }
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
