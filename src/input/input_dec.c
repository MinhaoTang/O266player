/*****************************************************************************
 * input_dec.c: Functions for the management of decoders
 *****************************************************************************
 * Copyright (C) 1999, 2000 VideoLAN
 * $Id: input_dec.c,v 1.1 2000/12/21 19:24:27 massiot Exp $
 *
 * Authors: Christophe Massiot <massiot@via.ecp.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/
/* FIXME: we shouldn't be obliged to include these */
#include "defs.h"

#include "config.h"
#include "common.h"
#include "threads.h"
#include "mtime.h"

#include "stream_control.h"
#include "input_ext-dec.h"

/*****************************************************************************
 * input_RunDecoder: spawns a new decoder thread
 *****************************************************************************/
vlc_thread_t input_RunDecoder( decoder_capabilities_t * p_decoder,
                               void * p_data )
{
    return( p_decoder->pf_create_thread( p_data ) );
}

/*****************************************************************************
 * input_EndDecoder: kills a decoder thread and waits until it's finished
 *****************************************************************************/
void input_EndDecoder( decoder_fifo_t * p_decoder_fifo, vlc_thread_t thread_id )
{
    p_decoder_fifo->b_die = 1;

    /* Make sure the thread leaves the NextDataPacket() function */
    vlc_mutex_lock( &p_decoder_fifo->data_lock);
    vlc_cond_signal( &p_decoder_fifo->data_wait );
    vlc_mutex_unlock( &p_decoder_fifo->data_lock );

    /* Waiting for the thread to exit */
    vlc_thread_join( thread_id );

    /* Freeing all packets still in the decoder fifo. */
    while( !DECODER_FIFO_ISEMPTY( *p_decoder_fifo ) )
    {
        p_decoder_fifo->pf_delete_pes( p_decoder_fifo->p_packets_mgt,
                                       DECODER_FIFO_START( *p_decoder_fifo ) );
        DECODER_FIFO_INCSTART( *p_decoder_fifo );
    }
}

