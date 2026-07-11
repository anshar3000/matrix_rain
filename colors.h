#ifndef COLORS_H
#define COLORS_H

#define BLACK_ATTRIBUTE 1
#define GREEN_ATTRIBUTE 2
#define WHITE_ATTRIBUTE 3

typedef struct {
    unsigned int color_pair;
    unsigned int attribute;
} attribute_pair;

void         init_colors(void);
attribute_pair get_attr_pair_given_dist(int dist_from_head, int length);

#endif /* COLORS_H */
