//=======================================
//		绘制sprite用到的汇编程序片断
//=======================================

//#define		read_alpha_2_ebx_run_length_2_eax		    \
//				{	xor		ebx, ebx			}	    \
//				{	mov		bx,	 word ptr[esi]	}	    \
//				{	xor		eax, eax			}	    \
//				{	add		esi, 2   			}       \
//				{	mov		al,  bl	            }	    \
//				{	shr		ebx, 8	            }


#define		read_alpha_2_ebx_run_length_2_eax		    \
				{	movzx	eax, byte ptr[esi]	}	    \
				{	movzx	ebx, byte ptr[esi + 1]	}	    \
				{	add		esi, 2   			}


#define		copy_pixel_use_eax	/*ebx指向调色板*/	\
				{	xor	    eax, eax            }	\
				{	mov	    al, byte ptr [esi]	}	\
				{	add		esi, 1 				}	\
				{	mov		ax, [ebx + eax * 2]	}	\
				{	add		edi, 2				}   \
				{	mov		[edi - 2], ax			}

#define		copy_4pixel_use_eax	/*ebx指向调色板*/	\
				{	movd    mm4, [esi]	        } 	\
				{	add		esi, 4 				}	\
                {   movd    eax, mm4            }   \
                {   and     eax, 0xff           }   \
				{	mov		ax, [ebx + eax * 2]	}	\
				{	add		edi, 2				}   \
				{	mov		[edi - 2], ax		}   \
                {   movd    eax, mm4            }   \
                {   shr     eax, 8              }   \
                {   and     eax, 0xff           }   \
				{	mov		ax, [ebx + eax * 2]	}	\
				{	add		edi, 2				}   \
				{	mov		[edi - 2], ax		}   \
                {   movd    eax, mm4            }   \
                {   shr     eax, 16             }   \
                {   and     eax, 0xff           }   \
				{	mov		ax, [ebx + eax * 2]	}	\
				{	add		edi, 2				}   \
				{	mov		[edi - 2], ax		}   \
                {   movd    eax, mm4            }   \
                {   shr     eax, 24             }   \
                {   and     eax, 0xff           }   \
				{	mov		ax, [ebx + eax * 2]	}	\
				{	add		edi, 2				}   \
				{	mov		[edi - 2], ax		}



#define		mix_2_pixel_color_use_eabdx									\
				{	movd	mm7, ecx			}						\
                {   xor     eax, eax            }                       \
				{	movd    ebx, mm0    		}	/* pPalette */		\
				{	mov	    al, byte ptr[esi]	}						\
				{	inc		esi					}						\
				{	mov     dx, [ebx + eax * 2]	}	/*edx = ...rgb*/	\
				{	movd	ecx, mm2    		}	/* nMask32 */		\
				{	mov		ax, dx				}	/*eax = ...rgb*/	\
				{	shl		eax, 16				}	/*eax = rgb...*/	\
				{	mov		ax, dx				}	/*eax = rgbrgb*/	\
				{	mov		dx, [edi]			}	/*edx = ...rgb*/	\
				{	and		eax, ecx			}	/*eax = .g.r.b*/	\
				{	mov		bx, dx				}	/*ebx = ...rgb*/	\
				{	shl		ebx, 16				}	/*ebx = rgb...*/	\
				{	mov		bx, dx				}	/*ebx = rgbrgb*/	\
				{	and		ebx, ecx			}	/*ebx = .g.r.b*/	\
                {   lea     edx, [ebx + ebx * 2]}                       \
                {   add     eax, edx            }                       \
				{	shr		eax, 2				}	/*c = (3xc1+c2)/4*/	\
				{	and     eax, ecx			}	/*eax = .g.r.b*/	\
				{	mov     dx, ax				}	/*edx = ...r.b*/	\
				{	shr     eax, 16				}	/*eax = ....g.*/	\
				{	add 	edi, 2				}						\
				{	or      ax, dx				}	/*eax = ...rgb*/	\
				{	movd     ecx, mm7			}                       \
				{	mov		[edi - 2], ax		}

#define		mix_2_pixel_color_alpha_use_eabdx							\
				{	movd	mm7, ecx    		}						\
				{	xor	    eax, eax    		}						\
				{	movd    ebx, mm0    		}	/* pPalette */		\
				{	mov  	al, byte ptr[esi]	}						\
				{	inc		esi					}						\
				{	mov     dx, [ebx + eax * 2]	}	/*edx = ...rgb*/	\
				{	movd	ecx, mm2    		}   /* nMask32 */		\
				{	mov		ax, dx				}	/*eax = ...rgb*/	\
				{	shl		eax, 16				}	/*eax = rgb...*/	\
				{	mov		ax, dx				}	/*eax = rgbrgb*/	\
				{	and		eax, ecx			}	/*eax = .g.r.b*/	\
				{	mov		dx, [edi]			}	/*edx = ...rgb*/	\
				{	mov		bx, dx				}	/*ebx = ...rgb*/	\
				{	shl		ebx, 16				}	/*ebx = rgb...*/	\
				{	mov		bx, dx				}	/*ebx = rgbrgb*/	\
				{	movd	edx, mm3			}	/*	nAlpha    */	\
				{	and		ebx, ecx			}	/*ebx = .g.r.b*/	\
                {   sub     eax, ebx            }   /*eax = c1 - c2*/   \
				{	imul    eax, edx			}	/*eax = (c1 - c2)*nAlpha */	\
				{	shr		eax, 5				}	/*c=(c1 - c2)*nAlpha/32*/ \
                {   add     eax, ebx            }   /*c=(c1 - c2)*nAlpha/32 + c2*/ \
				{	and     eax, ecx			}	/*eax = .g.r.b*/	\
				{	mov     dx, ax				}	/*edx = ...r.b*/	\
				{	shr     eax, 16				}	/*eax = ....g.*/	\
				{	add		edi, 2				}						\
				{	or      ax, dx				}	/*eax = ...rgb*/	\
				{	movd    ecx, mm7    		}                       \
				{	mov		[edi - 2], ax		}

#define		mix_2_pixel_color_alpha_use_eabdx_old						\
				{	movd	mm7, ecx    		}						\
				{	xor	    eax, eax    		}						\
				{	movd    ebx, mm0    		}	/* pPalette */		\
				{	mov  	al, byte ptr[esi]	}						\
				{	inc		esi					}						\
				{	mov     dx, [ebx + eax * 2]	}	/*edx = ...rgb*/	\
				{	movd	ecx, mm2    		}   /* nMask32 */		\
				{	mov		ax, dx				}	/*eax = ...rgb*/	\
				{	shl		eax, 16				}	/*eax = rgb...*/	\
				{	mov		ax, dx				}	/*eax = rgbrgb*/	\
				{	and		eax, ecx			}	/*eax = .g.r.b*/	\
				{	mov		dx, [edi]			}	/*edx = ...rgb*/	\
				{	mov		bx, dx				}	/*ebx = ...rgb*/	\
				{	shl		ebx, 16				}	/*ebx = rgb...*/	\
				{	mov		bx, dx				}	/*ebx = rgbrgb*/	\
				{	movd	edx, mm3			}	/*	nAlpha    */	\
				{	and		ebx, ecx			}	/*ebx = .g.r.b*/	\
				{	imul    eax, edx			}	/*eax = c1*nAlpha*/	\
				{	movd    edx, mm4			}	/* 32 - nAlpha */	\
				{	imul    ebx, edx			} /*ebx=c2*(32-nAlpha)*/\
				{	add		eax, ebx			}						\
				{	shr		eax, 5				}	/*c=(c1*nAlpha+c2*(32-nAlpha))/32*/ \
				{	and     eax, ecx			}	/*eax = .g.r.b*/	\
				{	mov     dx, ax				}	/*edx = ...r.b*/	\
				{	shr     eax, 16				}	/*eax = ....g.*/	\
				{	add		edi, 2				}						\
				{	or      ax, dx				}	/*eax = ...rgb*/	\
				{	movd    ecx, mm7    		}                       \
				{	mov		[edi - 2], ax		}
