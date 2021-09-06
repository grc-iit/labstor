// SPDX-License-Identifier: GPL-2.0
//#include <linux/pagewalk.h>
//#include <linux/highmem.h>
//#include <linux/sched.h>

#include <linux/mm.h>

struct mm_walk;

/**
 * mm_walk_ops - callbacks for walk_page_range
 * @pud_entry:		if set, called for each non-empty PUD (2nd-level) entry
 *			this handler should only handle pud_trans_huge() puds.
 *			the pmd_entry or pte_entry callbacks will be used for
 *			regular PUDs.
 * @pmd_entry:		if set, called for each non-empty PMD (3rd-level) entry
 *			this handler is required to be able to handle
 *			pmd_trans_huge() pmds.  They may simply choose to
 *			split_huge_page() instead of handling it explicitly.
 * @pte_entry:		if set, called for each non-empty PTE (4th-level) entry
 * @pte_hole:		if set, called for each hole at all levels
 * @hugetlb_entry:	if set, called for each hugetlb entry
 * @test_walk:		caller specific callback function to determine whether
 *			we walk over the current vma or not. Returning 0 means
 *			"do page table walk over the current vma", returning
 *			a negative value means "abort current page table walk
 *			right now" and returning 1 means "skip the current vma"
 */
struct mm_walk_ops {
    int (*pud_entry)(pud_t *pud, unsigned long addr,
                     unsigned long next, struct mm_walk *walk);
    int (*pmd_entry)(pmd_t *pmd, unsigned long addr,
                     unsigned long next, struct mm_walk *walk);
    int (*pte_entry)(pte_t *pte, unsigned long addr,
                     unsigned long next, struct mm_walk *walk);
    int (*pte_hole)(unsigned long addr, unsigned long next,
                    struct mm_walk *walk);
    int (*hugetlb_entry)(pte_t *pte, unsigned long hmask,
                         unsigned long addr, unsigned long next,
                         struct mm_walk *walk);
    int (*test_walk)(unsigned long addr, unsigned long next,
                     struct mm_walk *walk);
};

/**
 * mm_walk - walk_page_range data
 * @ops:	operation to call during the walk
 * @mm:		mm_struct representing the target process of page table walk
 * @vma:	vma currently walked (NULL if walking outside vmas)
 * @private:	private data for callbacks' usage
 *
 * (see the comment on walk_page_range() for more details)
 */
struct mm_walk {
    const struct mm_walk_ops *ops;
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    void *private;
};




#ifdef CONFIG_HUGETLB_PAGE

static inline bool is_vm_hugetlb_page(struct vm_area_struct *vma)
{
	return !!(vma->vm_flags & VM_HUGETLB);
}

#else

static inline bool is_vm_hugetlb_page(struct vm_area_struct *vma)
{
    return false;
}

#endif

static int walk_pte_range(pmd_t *pmd, unsigned long addr, unsigned long end,
                          struct mm_walk *walk)
{
    pte_t *pte;
    int err = 0;
    const struct mm_walk_ops *ops = walk->ops;

    pte = pte_offset_map(pmd, addr);
    for (;;) {
        err = ops->pte_entry(pte, addr, addr + PAGE_SIZE, walk);
        if (err)
            break;
        if (addr >= end - PAGE_SIZE)
            break;
        addr += PAGE_SIZE;
        pte++;
    }

    pte_unmap(pte);
    return err;
}

static int walk_pmd_range(pud_t *pud, unsigned long addr, unsigned long end,
                          struct mm_walk *walk)
{
    pmd_t *pmd;
    unsigned long next;
    const struct mm_walk_ops *ops = walk->ops;
    int err = 0;

    pmd = pmd_offset(pud, addr);
    do {
        again:
        next = pmd_addr_end(addr, end);
        if (pmd_none(*pmd) || !walk->vma) {
            if (ops->pte_hole)
                err = ops->pte_hole(addr, next, walk);
            if (err)
                break;
            continue;
        }
        /*
         * This implies that each ->pmd_entry() handler
         * needs to know about pmd_trans_huge() pmds
         */
        if (ops->pmd_entry)
            err = ops->pmd_entry(pmd, addr, next, walk);
        if (err)
            break;

        /*
         * Check this here so we only break down trans_huge
         * pages when we _need_ to
         */
        if (!ops->pte_entry)
            continue;

        split_huge_pmd(walk->vma, pmd, addr);
        if (pmd_trans_unstable(pmd))
            goto again;
        err = walk_pte_range(pmd, addr, next, walk);
        if (err)
            break;
    } while (pmd++, addr = next, addr != end);

    return err;
}

static int walk_pud_range(p4d_t *p4d, unsigned long addr, unsigned long end,
                          struct mm_walk *walk)
{
    pud_t *pud;
    unsigned long next;
    const struct mm_walk_ops *ops = walk->ops;
    int err = 0;

    pud = pud_offset(p4d, addr);
    do {
        again:
        next = pud_addr_end(addr, end);
        if (pud_none(*pud) || !walk->vma) {
            if (ops->pte_hole)
                err = ops->pte_hole(addr, next, walk);
            if (err)
                break;
            continue;
        }

        if (ops->pud_entry) {
            spinlock_t *ptl = pud_trans_huge_lock(pud, walk->vma);

            if (ptl) {
                err = ops->pud_entry(pud, addr, next, walk);
                spin_unlock(ptl);
                if (err)
                    break;
                continue;
            }
        }

        split_huge_pud(walk->vma, pud, addr);
        if (pud_none(*pud))
            goto again;

        if (ops->pmd_entry || ops->pte_entry)
            err = walk_pmd_range(pud, addr, next, walk);
        if (err)
            break;
    } while (pud++, addr = next, addr != end);

    return err;
}

static int walk_p4d_range(pgd_t *pgd, unsigned long addr, unsigned long end,
                          struct mm_walk *walk)
{
    p4d_t *p4d;
    unsigned long next;
    const struct mm_walk_ops *ops = walk->ops;
    int err = 0;

    p4d = p4d_offset(pgd, addr);
    do {
        next = p4d_addr_end(addr, end);
        if (p4d_none_or_clear_bad(p4d)) {
            if (ops->pte_hole)
                err = ops->pte_hole(addr, next, walk);
            if (err)
                break;
            continue;
        }
        if (ops->pmd_entry || ops->pte_entry)
            err = walk_pud_range(p4d, addr, next, walk);
        if (err)
            break;
    } while (p4d++, addr = next, addr != end);

    return err;
}

static int walk_pgd_range(unsigned long addr, unsigned long end,
                          struct mm_walk *walk)
{
    pgd_t *pgd;
    unsigned long next;
    const struct mm_walk_ops *ops = walk->ops;
    int err = 0;

    pgd = pgd_offset(walk->mm, addr);
    do {
        next = pgd_addr_end(addr, end);
        if (pgd_none_or_clear_bad(pgd)) {
            if (ops->pte_hole)
                err = ops->pte_hole(addr, next, walk);
            if (err)
                break;
            continue;
        }
        if (ops->pmd_entry || ops->pte_entry)
            err = walk_p4d_range(pgd, addr, next, walk);
        if (err)
            break;
    } while (pgd++, addr = next, addr != end);

    return err;
}

/*
 * Decide whether we really walk over the current vma on [@start, @end)
 * or skip it via the returned value. Return 0 if we do walk over the
 * current vma, and return 1 if we skip the vma. Negative values means
 * error, where we abort the current walk.
 */
static int walk_page_test(unsigned long start, unsigned long end,
                          struct mm_walk *walk)
{
    struct vm_area_struct *vma = walk->vma;
    const struct mm_walk_ops *ops = walk->ops;

    if (ops->test_walk)
        return ops->test_walk(start, end, walk);

    /*
     * vma(VM_PFNMAP) doesn't have any valid struct pages behind VM_PFNMAP
     * range, so we don't walk over it as we do for normal vmas. However,
     * Some callers are interested in handling hole range and they don't
     * want to just ignore any single address range. Such users certainly
     * define their ->pte_hole() callbacks, so let's delegate them to handle
     * vma(VM_PFNMAP).
     */
    if (vma->vm_flags & VM_PFNMAP) {
        int err = 1;
        if (ops->pte_hole)
            err = ops->pte_hole(start, end, walk);
        return err ? err : 1;
    }
    return 0;
}

static int __walk_page_range(unsigned long start, unsigned long end,
                             struct mm_walk *walk)
{
    int err = 0;
    struct vm_area_struct *vma = walk->vma;

    if (vma && is_vm_hugetlb_page(vma)) {
        if (walk->ops->hugetlb_entry)
            printk("test_addrs: HUGETLB for some reason...\n");
    } else
        err = walk_pgd_range(start, end, walk);

    return err;
}

/**
 * walk_page_range - walk page table with caller specific callbacks
 * @mm:		mm_struct representing the target process of page table walk
 * @start:	start address of the virtual address range
 * @end:	end address of the virtual address range
 * @ops:	operation to call during the walk
 * @private:	private data for callbacks' usage
 *
 * Recursively walk the page table tree of the process represented by @mm
 * within the virtual address range [@start, @end). During walking, we can do
 * some caller-specific works for each entry, by setting up pmd_entry(),
 * pte_entry(), and/or hugetlb_entry(). If you don't set up for some of these
 * callbacks, the associated entries/pages are just ignored.
 * The return values of these callbacks are commonly defined like below:
 *
 *  - 0  : succeeded to handle the current entry, and if you don't reach the
 *         end address yet, continue to walk.
 *  - >0 : succeeded to handle the current entry, and return to the caller
 *         with caller specific value.
 *  - <0 : failed to handle the current entry, and return to the caller
 *         with error code.
 *
 * Before starting to walk page table, some callers want to check whether
 * they really want to walk over the current vma, typically by checking
 * its vm_flags. walk_page_test() and @ops->test_walk() are used for this
 * purpose.
 *
 * struct mm_walk keeps current values of some common data like vma and pmd,
 * which are useful for the access from callbacks. If you want to pass some
 * caller-specific data to callbacks, @private should be helpful.
 *
 * Locking:
 *   Callers of walk_page_range() and walk_page_vma() should hold @mm->mmap_sem,
 *   because these function traverse vma list and/or access to vma's data.
 */
int walk_page_range(struct mm_struct *mm, unsigned long start,
                    unsigned long end, const struct mm_walk_ops *ops,
                    void *private)
{
    int err = 0;
    unsigned long next;
    struct vm_area_struct *vma;
    struct mm_walk walk = {
            .ops		= ops,
            .mm		= mm,
            .private	= private,
    };

    if (start >= end)
        return -EINVAL;

    if (!walk.mm)
        return -EINVAL;

    lockdep_assert_held(&walk.mm->mmap_sem);

    vma = find_vma(walk.mm, start);
    do {
        if (!vma) { /* after the last vma */
            walk.vma = NULL;
            next = end;
        } else if (start < vma->vm_start) { /* outside vma */
            walk.vma = NULL;
            next = min(end, vma->vm_start);
        } else { /* inside vma */
            walk.vma = vma;
            next = min(end, vma->vm_end);
            vma = vma->vm_next;

            err = walk_page_test(start, next, &walk);
            if (err > 0) {
                /*
                 * positive return values are purely for
                 * controlling the pagewalk, so should never
                 * be passed to the callers.
                 */
                err = 0;
                continue;
            }
            if (err < 0)
                break;
        }
        if (walk.vma || walk.ops->pte_hole)
            err = __walk_page_range(start, next, &walk);
        if (err)
            break;
    } while (start = next, start < end);
    return err;
}

int walk_page_vma(struct vm_area_struct *vma, const struct mm_walk_ops *ops,
                  void *private)
{
    struct mm_walk walk = {
            .ops		= ops,
            .mm		= vma->vm_mm,
            .vma		= vma,
            .private	= private,
    };
    int err;

    if (!walk.mm)
        return -EINVAL;

    lockdep_assert_held(&walk.mm->mmap_sem);

    err = walk_page_test(vma->vm_start, vma->vm_end, &walk);
    if (err > 0)
        return 0;
    if (err < 0)
        return err;
    return __walk_page_range(vma->vm_start, vma->vm_end, &walk);
}
