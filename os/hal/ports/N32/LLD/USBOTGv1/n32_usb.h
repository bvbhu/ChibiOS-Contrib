/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio
    ChibiOS - Copyright (C) 2023..2024 HorrorTroll
    ChibiOS - Copyright (C) 2023..2024 Zhaqian

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    USBOTGv1/n32_usb.h
 * @brief   N32H48x USBHS OTG registers layout header.
 *
 * @addtogroup USB
 * @{
 */

#ifndef N32_USB_H
#define N32_USB_H

/**
 * @brief   USBHS FIFO memory size in words.
 */
#define N32_USBHS_FIFO_MEM_SIZE         1280

/*===========================================================================*/
/* Register layout: N32H48x uses its own field names (DINEPCTRL etc.) which  */
/* differ from the generic DWC OTG names used by the driver code, so we     */
/* define a struct with the generic names mapped onto the N32H48x layout.   */
/*===========================================================================*/

/**
 * @brief   Host channel registers group.
 */
typedef struct {
  volatile uint32_t HCCHAR;     /**< @brief Host channel characteristics
                                          register (HCHCTRL).               */
  volatile uint32_t HCSPLT;     /**< @brief Host channel split register
                                          (HCSCTRL).                         */
  volatile uint32_t HCINT;      /**< @brief Host channel interrupt register
                                          (HCHINTSTS).                       */
  volatile uint32_t HCINTMSK;   /**< @brief Host channel interrupt mask
                                          register (HCHINTEN).               */
  volatile uint32_t HCTSIZ;     /**< @brief Host channel transfer size
                                          register (HCHTXSIZ).               */
  volatile uint32_t HCDMA;      /**< @brief Host channel DMA address
                                          register (HCHDMADD).               */
  volatile uint32_t resvd18;
  volatile uint32_t resvd1c;
} n32_usb_host_chn_t;

/**
 * @brief   Device input endpoint registers group.
 * @note    The N32H48x names are DINEPCTRL, DINEPINTSTS, DINEPTXSIZ,
 *          DINEPDMADD, DINEPTXFSTS (offsets 0x00/0x08/0x10/0x14/0x18).
 */
typedef struct {
  volatile uint32_t DIEPCTL;    /**< @brief Device control IN endpoint
                                          control register (DINEPCTRL).     */
  volatile uint32_t resvd4;
  volatile uint32_t DIEPINT;    /**< @brief Device IN endpoint interrupt
                                          register (DINEPINTSTS).            */
  volatile uint32_t resvdC;
  volatile uint32_t DIEPTSIZ;   /**< @brief Device IN endpoint transfer size
                                          register (DINEPTXSIZ).             */
  volatile uint32_t DIEPDMA;    /**< @brief Device IN endpoint DMA address
                                          register (DINEPDMADD).             */
  volatile uint32_t DTXFSTS;    /**< @brief Device IN endpoint transmit FIFO
                                          status register (DINEPTXFSTS).     */
  volatile uint32_t resvd1C;
} n32_usb_in_ep_t;

/**
 * @brief   Device output endpoint registers group.
 * @note    The N32H48x names are DOUTEPCTRL, DOUTEPINTSTS, DOUTEPTXSIZ,
 *          DOUTEPDMADD (offsets 0x00/0x08/0x10/0x14).
 */
typedef struct {
  volatile uint32_t DOEPCTL;    /**< @brief Device control OUT endpoint
                                          control register (DOUTEPCTRL).    */
  volatile uint32_t resvd4;
  volatile uint32_t DOEPINT;    /**< @brief Device OUT endpoint interrupt
                                          register (DOUTEPINTSTS).           */
  volatile uint32_t resvdC;
  volatile uint32_t DOEPTSIZ;   /**< @brief Device OUT endpoint transfer
                                          size register (DOUTEPTXSIZ).       */
  volatile uint32_t DOEPDMA;    /**< @brief Device OUT endpoint DMA address
                                          register (DOUTEPDMADD).            */
  volatile uint32_t resvd18;
  volatile uint32_t resvd1C;
} n32_usb_out_ep_t;

/**
 * @brief   USB registers memory map (USBHS at AHBPERIPH_BASE + 0x20000).
 */
typedef struct {
  volatile uint32_t GOTGCTL;    /**< @brief OTG control and status register
                                          (GCTRLSTS).                        */
  volatile uint32_t GOTGINT;    /**< @brief OTG interrupt register (unused). */
  volatile uint32_t GAHBCFG;    /**< @brief AHB configuration register.     */
  volatile uint32_t GUSBCFG;    /**< @brief USB configuration register
                                          (GCFG).                            */
  volatile uint32_t GRSTCTL;    /**< @brief Reset register (GRSTCTRL).      */
  volatile uint32_t GINTSTS;    /**< @brief Interrupt register.             */
  volatile uint32_t GINTMSK;    /**< @brief Interrupt mask register
                                          (GINTEN).                          */
  volatile uint32_t GRXSTSR;    /**< @brief Receive status debug read
                                          register (GRXSTS).                 */
  volatile uint32_t GRXSTSP;    /**< @brief Receive status read/pop
                                          register.                          */
  volatile uint32_t GRXFSIZ;    /**< @brief Receive FIFO size register.     */
  volatile uint32_t DIEPTXF0;   /**< @brief Endpoint 0 transmit FIFO size
                                          register (GNPTXFSIZ_DINEP0TXFSIZ). */
  volatile uint32_t GNPTXSTS;   /**< @brief Non-periodic TxFIFO size/request
                                          queue status register (GNPTXFSTS).*/
  volatile uint32_t resvd30;
  volatile uint32_t resvd34;
  volatile uint32_t GCCFG;      /**< @brief General controller config
                                          (unused on N32H48x).               */
  volatile uint32_t GUID;       /**< @brief Controller ID register (CID).   */
  volatile uint32_t resvd58[48];
  volatile uint32_t HPTXFSIZ;   /**< @brief Host periodic transmit FIFO size
                                          register.                          */
  volatile uint32_t DIEPTXF[8]; /**< @brief Device IN endpoint transmit FIFO
                                          size registers (DINEPPTXFSIZ[8]).  */
  volatile uint32_t resvd128[8];
  volatile uint32_t resvd140[176];
  volatile uint32_t HCFG;       /**< @brief Host configuration register.    */
  volatile uint32_t HFIR;       /**< @brief Host frame interval register
                                          (HFRI).                            */
  volatile uint32_t HFNUM;      /**< @brief Host frame number/frame time
                                          Remaining register.                */
  volatile uint32_t resvd40C;
  volatile uint32_t HPTXSTS;    /**< @brief Host periodic transmit FIFO/queue
                                          status register (HPTXFQSTS).       */
  volatile uint32_t HAINT;      /**< @brief Host all channels interrupt
                                          register (HACHINT).                */
  volatile uint32_t HAINTMSK;   /**< @brief Host all channels interrupt mask
                                          register (HACHINTEN).              */
  volatile uint32_t resvd41C[9];
  volatile uint32_t HPRT;       /**< @brief Host port control and status
                                          register (HPCS).                   */
  volatile uint32_t resvd444[47];
  n32_usb_host_chn_t hc[16];    /**< @brief Host channels array.            */
  volatile uint32_t resvd700[64];
  volatile uint32_t DCFG;       /**< @brief Device configuration register.  */
  volatile uint32_t DCTL;       /**< @brief Device control register (DCTRL).*/
  volatile uint32_t DSTS;       /**< @brief Device status register.         */
  volatile uint32_t resvd80C;
  volatile uint32_t DIEPMSK;    /**< @brief Device IN endpoint common
                                          interrupt mask register
                                          (DINEPINTEN).                      */
  volatile uint32_t DOEPMSK;    /**< @brief Device OUT endpoint common
                                          interrupt mask register
                                          (DOUTEPINTEN).                     */
  volatile uint32_t DAINT;      /**< @brief Device all endpoints interrupt
                                          register (DAEPINTSTS).             */
  volatile uint32_t DAINTMSK;   /**< @brief Device all endpoints interrupt
                                          mask register (DAEPINTEN).         */
  volatile uint32_t resvd820;
  volatile uint32_t resvd824;
  volatile uint32_t resvd828;
  volatile uint32_t resvd82C;
  volatile uint32_t resvd830;
  volatile uint32_t DIEPEMPMSK; /**< @brief Device IN endpoint FIFO empty
                                          interrupt mask register
                                          (DINEPFEINTEN).                    */
  volatile uint32_t DEACHINT;   /**< @brief Device all endpoints interrupt
                                          register (DEEPINTSTS).             */
  volatile uint32_t DEACHINTMSK; /**< @brief Device all endpoints interrupt
                                          mask register (DEEPINTEN).         */
  volatile uint32_t resvd840;
  volatile uint32_t DIEPEACHMSK1; /**< @brief Device IN endpoint 1
                                               interrupt mask register
                                               (DINEPXINTEN[0]).             */
  volatile uint32_t resvd848[14];
  volatile uint32_t resvd880;
  volatile uint32_t DOEPEACHMSK1; /**< @brief Device OUT endpoint 1
                                               interrupt mask register
                                               (DOUTEPXINTEN[0]).            */
  volatile uint32_t resvd888[14];
  volatile uint32_t resvd8C0[16];
  n32_usb_in_ep_t ie[16];       /**< @brief Input endpoints.                */
  n32_usb_out_ep_t oe[16];      /**< @brief Output endpoints.               */
  volatile uint32_t resvdD00[64];
  volatile uint32_t PCGCCTL;    /**< @brief Power and clock gating control
                                          register (PWRCTRL).                */
  volatile uint32_t resvdE04[127];
  volatile uint32_t FIFO[16][1024];
} n32_otg_t;

/*===========================================================================*/
/* Register bit definitions: mapped onto the N32H48x CMSIS header names     */
/* (USBHS_*). The generic DWC names are defined as aliases so the driver    */
/* code can use the standard names.                                         */
/*===========================================================================*/

/* GOTGCTL (GCTRLSTS). */
#define GOTGCTL_CURMOD          USBHS_GCTRLSTS_CMODE
#define GOTGCTL_CONIDSTS        USBHS_GCTRLSTS_IDSTS

/* GUSBCFG (GCFG). */
#define GUSBCFG_FDEVMODE        USBHS_GCFG_FDMODE
#define GUSBCFG_FHSTMODE        USBHS_GCFG_FHMODE
#define GUSBCFG_PHYSEL          USBHS_GCFG_PHYSEL
#define GUSBCFG_USBTRDTIM_MASK  USBHS_GCFG_TRDTIM
#define GUSBCFG_USBTRDTIM(n)    (((n) << USBHS_GCFG_TRDTIM_POS) & USBHS_GCFG_TRDTIM)

/* GAHBCFG (GAHBCFG). */
#define GAHBCFG_GLBINTMSK       USBHS_GAHBCFG_GINTEN

/* GRSTCTL (GRSTCTRL). */
#define GRSTCTL_AHBIDLE         USBHS_GRSTCTRL_AHBIDLE
#define GRSTCTL_TXFNUM_MASK     USBHS_GRSTCTRL_TXFNUM
#define GRSTCTL_TXFNUM(n)       (((n) << USBHS_GRSTCTRL_TXFNUM_POS) & USBHS_GRSTCTRL_TXFNUM)
#define GRSTCTL_TXFFLSH         USBHS_GRSTCTRL_TXFFLSH
#define GRSTCTL_RXFFLSH         USBHS_GRSTCTRL_RXFFLSH
#define GRSTCTL_FRMCNTRST       USBHS_GRSTCTRL_HFCRST
#define GRSTCTL_PIUSFTRST       USBHS_GRSTCTRL_PFSSRST
#define GRSTCTL_CSFTRST         USBHS_GRSTCTRL_CSRST

/* GINTSTS (GINTSTS). */
#define GINTSTS_WKUPINT         USBHS_GINTSTS_WKUPIF
#define GINTSTS_DISCONINT       USBHS_GINTSTS_DISCIF
#define GINTSTS_CONIDSCHG       USBHS_GINTSTS_IDSTSCIF
#define GINTSTS_PTXFEMP         USBHS_GINTSTS_PTXFEIF
#define GINTSTS_HCHINT          USBHS_GINTSTS_HCHIF
#define GINTSTS_PRTINT          USBHS_GINTSTS_HPIF
#define GINTSTS_INCOMPIP        USBHS_GINTSTS_PTNCIF_ISOUTNCIF
#define GINTSTS_INCOMPISOOUT    USBHS_GINTSTS_PTNCIF_ISOUTNCIF
#define GINTSTS_INCOMPISOIN     USBHS_GINTSTS_ISOINCIF
#define GINTSTS_OEPTINT         USBHS_GINTSTS_OUTEPIF
#define GINTSTS_IEPTINT         USBHS_GINTSTS_INEPIF
#define GINTSTS_EOPF            USBHS_GINTSTS_EOPFIF
#define GINTSTS_ISOOUTDROP      USBHS_GINTSTS_ISOUTPDIF
#define GINTSTS_ENUMDONE        USBHS_GINTSTS_ENUMDIF
#define GINTSTS_USBRST          USBHS_GINTSTS_USBRSTIF
#define GINTSTS_USBSUSP         USBHS_GINTSTS_USBSUSPIF
#define GINTSTS_ERLYSUSP        USBHS_GINTSTS_ESUSPIF
#define GINTSTS_GOUTNAKEFF      USBHS_GINTSTS_GOUTNAKEIF
#define GINTSTS_GINNAKEFF       USBHS_GINTSTS_GINNPNAKEIF
#define GINTSTS_NPTXFEMP        USBHS_GINTSTS_NPTXFEIF
#define GINTSTS_RXFLVL          USBHS_GINTSTS_RXFNEIF
#define GINTSTS_SOF             USBHS_GINTSTS_SOFIF
#define GINTSTS_OTGINT          (0U)
#define GINTSTS_MODEMIS         USBHS_GINTSTS_MODMISIF
#define GINTSTS_CURMOD          USBHS_GINTSTS_CMODE

/* GINTMSK (GINTEN). */
#define GINTMSK_WKUPINTMSK      USBHS_GINTEN_WKUPIEN
#define GINTMSK_DISCONINTMSK    USBHS_GINTEN_DISCIEN
#define GINTMSK_CONIDSCHGMSK    USBHS_GINTEN_IDSTSCIEN
#define GINTMSK_PTXFEMPMSK      USBHS_GINTEN_PTXFEIEN
#define GINTMSK_HCHINTMSK       USBHS_GINTEN_HCHIEN
#define GINTMSK_PRTINTMSK       USBHS_GINTEN_HPIEN
#define GINTMSK_INCOMPIPMSK     USBHS_GINTEN_PTNCIEN_ISOUTNCIEN
#define GINTMSK_INCOMPISOOUTMSK USBHS_GINTEN_PTNCIEN_ISOUTNCIEN
#define GINTMSK_INCOMISOINMSK   USBHS_GINTEN_ISOINCIEN
#define GINTMSK_OEPTINTMSK      USBHS_GINTEN_OUTEPIEN
#define GINTMSK_IEPTINTMSK      USBHS_GINTEN_INEPIEN
#define GINTMSK_EOPFMSK         USBHS_GINTEN_EOPFIEN
#define GINTMSK_ISOOUTDROPMSK   USBHS_GINTEN_ISOUTPDIEN
#define GINTMSK_ENUMDONEMSK     USBHS_GINTEN_ENUMDIEN
#define GINTMSK_USBRSTMSK       USBHS_GINTEN_USBRSTIEN
#define GINTMSK_USBSUSPMSK      USBHS_GINTEN_USBSUSPIEN
#define GINTMSK_ERLYSUSPMSK     USBHS_GINTEN_ESUSPIEN
#define GINTMSK_GOUTNAKEFFMSK   USBHS_GINTEN_GOUTNAKEIEN
#define GINTMSK_GINNAKEFFMSK    USBHS_GINTEN_GINNPNAKEIEN
#define GINTMSK_NPTXFEMPMSK     USBHS_GINTEN_NPTXFEIEN
#define GINTMSK_RXFLVLMSK       USBHS_GINTEN_RXFNEIEN
#define GINTMSK_SOFMSK          USBHS_GINTEN_SOFIEN
#define GINTMSK_OTGINTMSK       USBHS_GINTEN_USBHSIEN
#define GINTMSK_MODEMISMSK      USBHS_GINTEN_MODMISIEN

/* GRXSTSP (GRXSTSP). */
#define GRXSTSP_PKTSTS_MASK     USBHS_GRXSTS_PKTSTS
#define GRXSTSP_PKTSTS(n)       (((n) << USBHS_GRXSTS_PKTSTS_POS) & USBHS_GRXSTS_PKTSTS)
#define GRXSTSP_OUT_GLOBAL_NAK  GRXSTSP_PKTSTS(1)
#define GRXSTSP_OUT_DATA        GRXSTSP_PKTSTS(2)
#define GRXSTSP_OUT_COMP        GRXSTSP_PKTSTS(3)
#define GRXSTSP_SETUP_COMP      GRXSTSP_PKTSTS(4)
#define GRXSTSP_SETUP_DATA      GRXSTSP_PKTSTS(6)
#define GRXSTSP_DPID_MASK       USBHS_GRXSTS_DPID
#define GRXSTSP_DPID(n)         (((n) << USBHS_GRXSTS_DPID_POS) & USBHS_GRXSTS_DPID)
#define GRXSTSP_BCNT_MASK       USBHS_GRXSTS_BCNT
#define GRXSTSP_BCNT(n)         (((n) << USBHS_GRXSTS_BCNT_POS) & USBHS_GRXSTS_BCNT)
#define GRXSTSP_CHNUM_MASK      USBHS_GRXSTS_CHEPNUM
#define GRXSTSP_CHNUM(n)        (((n) << USBHS_GRXSTS_CHEPNUM_POS) & USBHS_GRXSTS_CHEPNUM)
#define GRXSTSP_EPTNUM_MASK     USBHS_GRXSTS_CHEPNUM
#define GRXSTSP_EPTNUM(n)       (((n) << USBHS_GRXSTS_CHEPNUM_POS) & USBHS_GRXSTS_CHEPNUM)
#define GRXSTSP_BCNT_OFF        USBHS_GRXSTS_BCNT_POS
#define GRXSTSP_EPTNUM_OFF      USBHS_GRXSTS_CHEPNUM_POS

/* GRXFSIZ (GRXFSIZ). */
#define GRXFSIZ_RXFDEP_MASK     USBHS_GRXFSIZ_RXFDEP
#define GRXFSIZ_RXFDEP(n)       ((n) & USBHS_GRXFSIZ_RXFDEP)

/* DIEPTXF0 (GNPTXFSIZ_DINEP0TXFSIZ). */
#define DIEPTXF_INEPTXFDEP_MASK USBHS_DINEP0TXSIZ_NPTXFDEP
#define DIEPTXF_INEPTXFDEP(n)   (((n) << USBHS_DINEP0TXSIZ_NPTXFDEP_POS) & USBHS_DINEP0TXSIZ_NPTXFDEP)
#define DIEPTXF_INEPTXFSTADDR_MASK USBHS_DINEP0TXSIZ_IEP0TXFRSADD
#define DIEPTXF_INEPTXFSTADDR(n) (((n) << USBHS_DINEP0TXSIZ_IEP0TXFRSADD_POS) & USBHS_DINEP0TXSIZ_IEP0TXFRSADD)

/* DCFG (DCFG). */
#define DCFG_PERFRINT_MASK      USBHS_DCFG_PFRITVL
#define DCFG_PERFRINT(n)        (((n) << USBHS_DCFG_PFRITVL_POS) & USBHS_DCFG_PFRITVL)
#define DCFG_DEVADDR_MASK       USBHS_DCFG_DEVARR
#define DCFG_DEVADDR(n)         (((n) << USBHS_DCFG_DEVARR_POS) & USBHS_DCFG_DEVARR)
#define DCFG_NZSTSOUTHSHK       USBHS_DCFG_NZLSOUTHSK
#define DCFG_DEVSPD_MASK        USBHS_DCFG_DEVSPD
#define DCFG_DEVSPD_HS          (0U)
#define DCFG_DEVSPD_HS_FS       (1U)
#define DCFG_DEVSPD_FS11        (3U)

/* DCTL (DCTRL). */
#define DCTL_PWROPRGDNE         USBHS_DCTRL_POPDNE
#define DCTL_CGOUTNAK           USBHS_DCTRL_CGONAK
#define DCTL_SGOUTNAK           USBHS_DCTRL_SGONAK
#define DCTL_CGNPINNAK          USBHS_DCTRL_CGNPINAK
#define DCTL_SGNPINNAK          USBHS_DCTRL_SGINAK
#define DCTL_TSTCTL_MASK        USBHS_DCTRL_TSCTRL
#define DCTL_TSTCTL(n)          (((n) << USBHS_DCTRL_TSCTRL_POS) & USBHS_DCTRL_TSCTRL)
#define DCTL_GOUTNAKSTS         USBHS_DCTRL_GONAKSTS
#define DCTL_GNPINNAKSTS        USBHS_DCTRL_GINAKSTS
#define DCTL_SFTDISCON          USBHS_DCTRL_SFTDIS
#define DCTL_RWKUPSIG           USBHS_DCTRL_RMWKUP

/* DSTS (DSTS). */
#define DSTS_SOFFN_MASK         USBHS_DSTS_SOFFN
#define DSTS_SOFFN(n)           (((n) << USBHS_DSTS_SOFFN_POS) & USBHS_DSTS_SOFFN)
#define DSTS_SOFFN_ODD          (1U << (USBHS_DSTS_SOFFN_POS + 0))
#define DSTS_ETICERR            USBHS_DSTS_ERERRF
#define DSTS_ENUMSPD_MASK       USBHS_DSTS_ENUMSPD
#define DSTS_ENUMSPD_FS_48      (3U)
#define DSTS_ENUMSPD_FS_30_60   (1U)
#define DSTS_ENUMSPD_HS_480     (0U)
#define DSTS_SUSPSTS            USBHS_DSTS_SUSPF

/* DIEPMSK (DINEPINTEN). */
#define DIEPMSK_TXFIFOUDRMSK    USBHS_DINEPINTEN_TXFUDIEN
#define DIEPMSK_INEPTNAKMSK     USBHS_DINEPINTEN_INEPNAKEIEN
#define DIEPMSK_INTKNTXFEMPMSK  USBHS_DINEPINTEN_TXFERINTKIEN
#define DIEPMSK_TIMEOUTMSK      USBHS_DINEPINTEN_TOIEN
#define DIEPMSK_EPTDISMSK       USBHS_DINEPINTEN_EPDIEN
#define DIEPMSK_XFERCMSK        USBHS_DINEPINTEN_TXCIEN

/* DOEPMSK (DOUTEPINTEN). */
#define DOEPMSK_OUTTEPDMSK      USBHS_DOUTEPINTEN_EPDISROTIEN
#define DOEPMSK_SETUPMSK        USBHS_DOUTEPINTEN_STUPDNEIEN
#define DOEPMSK_EPTDISMSK       USBHS_DOUTEPINTEN_EPDIEN
#define DOEPMSK_XFERCMSK        USBHS_DOUTEPINTEN_TXCIEN

/* DAINT (DAEPINTSTS). */
#define DAINT_OUTEPTINT_MASK    USBHS_DAEPINTSTS_OUTEPINT
#define DAINT_OUTEPTINT(n)      (((n) << USBHS_DAEPINTSTS_OUTEPINT_POS) & USBHS_DAEPINTSTS_OUTEPINT)
#define DAINT_INEPTINT_MASK     USBHS_DAEPINTSTS_IEPINT
#define DAINT_INEPTINT(n)       (((n) << USBHS_DAEPINTSTS_INEPINT_POS) & USBHS_DAEPINTSTS_IEPINT)

/* DAINTMSK (DAEPINTEN). */
#define DAINTMSK_OUTEPTMSK_MASK USBHS_DAEPINTEN_OUTEPIEN
#define DAINTMSK_OUTEPTMSK(n)   (1U << (16 + (n)))
#define DAINTMSK_INEPTMSK_MASK  USBHS_DAEPINTEN_INEPIEN
#define DAINTMSK_INEPTMSK(n)    (1U << (n))

/* DIEPEMPMSK (DINEPFEINTEN). */
#define DIEPEMPMSK_INEPTXFEMSK(n) (1U << (n))

/* DIEPCTL (DINEPCTRL). */
#define DIEPCTL_EPTENA          USBHS_DINEPCTRL_EPEN
#define DIEPCTL_EPTDIS          USBHS_DINEPCTRL_EPDIS
#define DIEPCTL_SETD1PID        USBHS_DINEPCTRL_SD1PID_SODDFRM
#define DIEPCTL_SETODDFR        USBHS_DINEPCTRL_SD1PID_SODDFRM
#define DIEPCTL_SETD0PID        USBHS_DINEPCTRL_SD0PID_SEVNFRM
#define DIEPCTL_SETEVENFR       USBHS_DINEPCTRL_SD0PID_SEVNFRM
#define DIEPCTL_SNAK            USBHS_DINEPCTRL_SNAK
#define DIEPCTL_CNAK            USBHS_DINEPCTRL_CNAK
#define DIEPCTL_TXFNUM_MASK     USBHS_DINEPCTRL_TXFNUM
#define DIEPCTL_TXFNUM(n)       (((n) << USBHS_DINEPCTRL_TXFNUM_POS) & USBHS_DINEPCTRL_TXFNUM)
#define DIEPCTL_STALL           USBHS_DINEPCTRL_STALL
#define DIEPCTL_EPTYPE_MASK     USBHS_DINEPCTRL_EPTYPE
#define DIEPCTL_EPTYPE_CTRL     (0U)
#define DIEPCTL_EPTYPE_ISO      (1U << USBHS_DINEPCTRL_EPTYPE_POS)
#define DIEPCTL_EPTYPE_BULK     (2U << USBHS_DINEPCTRL_EPTYPE_POS)
#define DIEPCTL_EPTYPE_INTR     (3U << USBHS_DINEPCTRL_EPTYPE_POS)
#define DIEPCTL_NAKSTS          USBHS_DINEPCTRL_NAKSTS
#define DIEPCTL_EOFRNUM         USBHS_DINEPCTRL_EPDPID_EPEOFRM
#define DIEPCTL_DPID            USBHS_DINEPCTRL_EPDPID_EPEOFRM
#define DIEPCTL_USBACEPT        USBHS_DINEPCTRL_EPACT
#define DIEPCTL_MPS_MASK        USBHS_DINEPCTRL_MPLEN
#define DIEPCTL_MPS(n)          ((n) & USBHS_DINEPCTRL_MPLEN)

/* DIEPINT (DINEPINTSTS). */
#define DIEPINT_TXFEMP          USBHS_DINEPINTSTS_TXFEIF
#define DIEPINT_INEPTNAK        USBHS_DINEPINTSTS_INEPNAKEIF
#define DIEPINT_INTKNTXFEMP     USBHS_DINEPINTSTS_TXFERINTIF
#define DIEPINT_TIMEOUT         USBHS_DINEPINTSTS_TOUTIF
#define DIEPINT_EPTDISD         USBHS_DINEPINTSTS_EPDISIF
#define DIEPINT_XFERC           USBHS_DINEPINTSTS_TXCIF

/* DIEPTSIZ (DINEPTXSIZ). */
#define DIEPTSIZ_MC_MASK        USBHS_DINEPTXSIZ_MCNT
#define DIEPTSIZ_MC(n)          (((n) << USBHS_DINEPTXSIZ_MCNT_POS) & USBHS_DINEPTXSIZ_MCNT)
#define DIEPTSIZ_PKTCNT_MASK    USBHS_DINEPTXSIZ_PKTCNT
#define DIEPTSIZ_PKTCNT(n)      (((n) << USBHS_DINEPTXSIZ_PKTCNT_POS) & USBHS_DINEPTXSIZ_PKTCNT)
#define DIEPTSIZ_XFERSIZE_MASK  USBHS_DINEPTXSIZ_TLEN
#define DIEPTSIZ_XFERSIZE(n)    ((n) & USBHS_DINEPTXSIZ_TLEN)

/* DTXFSTS (DINEPTXFSTS). */
#define DTXFSTS_INEPTXFSAV_MASK USBHS_DINEPTXFSTS_TXFSPCAVL

/* DOEPCTL (DOUTEPCTRL). */
#define DOEPCTL_EPTENA          USBHS_DOUTEPCTRL_EPEN
#define DOEPCTL_EPTDIS          USBHS_DOUTEPCTRL_EPDIS
#define DOEPCTL_SETD1PID        USBHS_DOUTEPCTRL_SD1PID_SODDFRM
#define DOEPCTL_SETODDFR        USBHS_DOUTEPCTRL_SD1PID_SODDFRM
#define DOEPCTL_SETD0PID        USBHS_DOUTEPCTRL_SD0PID_SEVNFRM
#define DOEPCTL_SETEVENFR       USBHS_DOUTEPCTRL_SD0PID_SEVNFRM
#define DOEPCTL_SNAK            USBHS_DOUTEPCTRL_SNAK
#define DOEPCTL_CNAK            USBHS_DOUTEPCTRL_CNAK
#define DOEPCTL_STALL           USBHS_DOUTEPCTRL_STALL
#define DOEPCTL_SNP             (1U << 20)
#define DOEPCTL_EPTYPE_MASK     USBHS_DOUTEPCTRL_EPTYPE
#define DOEPCTL_EPTYPE_CTRL     (0U)
#define DOEPCTL_EPTYPE_ISO      (1U << USBHS_DOUTEPCTRL_EPTYPE_POS)
#define DOEPCTL_EPTYPE_BULK     (2U << USBHS_DOUTEPCTRL_EPTYPE_POS)
#define DOEPCTL_EPTYPE_INTR     (3U << USBHS_DOUTEPCTRL_EPTYPE_POS)
#define DOEPCTL_NAKSTS          USBHS_DOUTEPCTRL_NAKSTS
#define DOEPCTL_EOFRNUM         USBHS_DOUTEPCTRL_EPDPID_EPEOFRM
#define DOEPCTL_DPID            USBHS_DOUTEPCTRL_EPDPID_EPEOFRM
#define DOEPCTL_USBACEPT        USBHS_DOUTEPCTRL_EPACT
#define DOEPCTL_MPS_MASK        USBHS_DOUTEPCTRL_MPLEN
#define DOEPCTL_MPS(n)          ((n) & USBHS_DOUTEPCTRL_MPLEN)

/* DOEPINT (DOUTEPINTSTS). */
#define DOEPINT_B2BSTUP         USBHS_DOUTEPINTSTS_B2BSTUPRIF
#define DOEPINT_OUTTEPD         USBHS_DOUTEPINTSTS_OUTTRXEPDISIF
#define DOEPINT_SETUP           USBHS_DOUTEPINTSTS_STUPPDNEIF
#define DOEPINT_EPTDISD         USBHS_DOUTEPINTSTS_EPDISIF
#define DOEPINT_XFERC           USBHS_DOUTEPINTSTS_TXCIF

/* DOEPTSIZ (DOUTEPTXSIZ). */
#define DOEPTSIZ_RXDPID_MASK    USBHS_DOUTEPTXSIZ_STUPPCNT_RXDPID
#define DOEPTSIZ_RXDPID(n)      (((n) << USBHS_DOUTEPTXSIZ_STUPPCNT_POS) & USBHS_DOUTEPTXSIZ_STUPPCNT_RXDPID)
#define DOEPTSIZ_SETUPCNT_MASK  USBHS_DOUTEPTXSIZ_STUPPCNT_RXDPID
#define DOEPTSIZ_SETUPCNT(n)    (((n) << USBHS_DOUTEPTXSIZ_STUPPCNT_POS) & USBHS_DOUTEPTXSIZ_STUPPCNT_RXDPID)
#define DOEPTSIZ_PKTCNT_MASK    USBHS_DOUTEPTXSIZ_PKTCNT
#define DOEPTSIZ_PKTCNT(n)      (((n) << USBHS_DOUTEPTXSIZ_PKTCNT_POS) & USBHS_DOUTEPTXSIZ_PKTCNT)
#define DOEPTSIZ_XFERSIZE_MASK  USBHS_DOUTEPTXSIZ_TLEN
#define DOEPTSIZ_XFERSIZE(n)    ((n) & USBHS_DOUTEPTXSIZ_TLEN)

/* PCGCCTL (PWRCTRL). */
#define PCGCCTL_SUSPENDM        USBHS_PWRCTRL_PHYSLEEP
#define PCGCCTL_STOPPCLK        USBHS_PWRCTRL_PHYSTP

/* FIFO memory address. */
#define N32_USBHS_BASE          ((n32_otg_t *)USBHS_BASE)

#endif /* N32_USB_H */

/** @} */
