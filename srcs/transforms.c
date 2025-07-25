/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   transforms.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluis-ya <rluis-ya@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 10:27:21 by rluis-ya          #+#    #+#             */
/*   Updated: 2025/07/17 18:26:16 by rluis-ya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fdf.h"

static
void	ft_identity(t_mat4 *id)
{
	unsigned int	i;

	i = 0;
	while (i < DIM)
		id->matrix[5 * i++] = MAT4_IDENTITY_VAL;
}

static
void	init_tables(t_trig_lookup *cache)
{
	int		i;
	float	rad;

	i = 0;
    while (i < TABLE_SIZE) 
	{
        rad = RAD_MIN + i * RAD_STEP;
		cache->sin_table[i] = sinf(rad);
		cache->cos_table[i] = cosf(rad);
		i++;
    }
}

static
float	fast_sin_cos(t_trig_lookup *cache, float rad, int flag)
{
	int index;

	rad = fmod(rad, RAD_MAX);
    if (rad < 0)
		rad += RAD_MAX;
	index = (int)((rad - RAD_MIN) / RAD_STEP);
	if (index >= TABLE_SIZE)
		index = TABLE_SIZE - 1;
	if (!flag)
		return cache->sin_table[index];
	else
		return cache->cos_table[index];
}

static
t_quat	quat_from_euler(float x_rad, float y_rad, float z_rad, t_trig_lookup *cache)
{
	t_quat				q;
	t_quaternion_const	k;

	k.half_x = x_rad * 0.5f;
	k.half_y = y_rad * 0.5f;
	k.half_z = z_rad * 0.5f;
	k.cx = fast_sin_cos(cache, k.half_x, 1);
	k.sx = fast_sin_cos(cache, k.half_x, 0);
	k.cy = fast_sin_cos(cache, k.half_y, 1);
	k.sy = fast_sin_cos(cache, k.half_y, 0);
	k.cz = fast_sin_cos(cache, k.half_z, 1);
	k.sz = fast_sin_cos(cache, k.half_z, 0);

    q.w = k.cx * k.cy * k.cz + k.sx * k.sy * k.sz;
    q.x = k.sx * k.cy * k.cz - k.cx * k.sy * k.sz;
    q.y = k.cx * k.sy * k.cz + k.sx * k.cy * k.sz;
    q.z = k.cx * k.cy * k.sz - k.sx * k.sy * k.cz;

    return (q);
}

void	mat4_from_quat(t_mat4 *m, t_quat q)
{
	float xx = q.x * q.x;
	float yy = q.y * q.y;
	float zz = q.z * q.z;
	float xy = q.x * q.y;
	float xz = q.x * q.z;
	float yz = q.y * q.z;
	float wx = q.w * q.x;
	float wy = q.w * q.y;
	float wz = q.w * q.z;

	m->matrix[0]  = 1.0f - 2.0f * (yy + zz);
	m->matrix[1]  = 2.0f * (xy - wz);
	m->matrix[2]  = 2.0f * (xz + wy);
	m->matrix[3]  = 0.0f;

	m->matrix[4]  = 2.0f * (xy + wz);
	m->matrix[5]  = 1.0f - 2.0f * (xx + zz);
	m->matrix[6]  = 2.0f * (yz - wx);
	m->matrix[7]  = 0.0f;

	m->matrix[8]  = 2.0f * (xz - wy);
	m->matrix[9]  = 2.0f * (yz + wx);
	m->matrix[10] = 1.0f - 2.0f * (xx + yy);
	m->matrix[11] = 0.0f;

	m->matrix[12] = 0.0f;
	m->matrix[13] = 0.0f;
	m->matrix[14] = 0.0f;
	m->matrix[15] = 1.0f;
}

static
void	ft_scale_matrix(t_mat4 *m, float sx, float sy, float sz)
{
	ft_memset(m, 0, sizeof(t_mat4));
	m->matrix[0] = sx;
	m->matrix[5] = sy;
	m->matrix[10] = sz;
	m->matrix[15] = 1.0f;
}

static
void	ft_translate_matrix(t_mat4 *m, float tx, float ty, float tz)
{
	ft_memset(m, 0, sizeof(t_mat4));
	m->matrix[0] = 1.0f;
	m->matrix[5] = 1.0f;
	m->matrix[10] = 1.0f;
	m->matrix[15] = 1.0f;
	m->matrix[12] = tx;
	m->matrix[13] = ty;
	m->matrix[14] = tz;
}

static
void	ft_matmul(t_mat4 *out, t_mat4 *a, t_mat4 *b)
{
	int		iter_row;
	int		iter_col;
	int		iter_k;

	iter_row = 0;
	while (iter_row < DIM)
	{
		iter_col = 0;
		while (iter_col < DIM)
		{
			iter_k = 0;
			while (iter_k < DIM)
			{
				out->matrix[iter_row * DIM + iter_col] += a->matrix[iter_row * DIM + iter_k] * b->matrix[iter_k * DIM + iter_col];
				iter_k++;
			}
			iter_col++;
		}
		iter_row++;
	}
}

static
void	ft_pipeline(t_mat4 *out, t_transform_vals obj, t_trig_lookup *cache)
{
	t_quat	q;
	t_mat4	scale;
	t_mat4	translate;
	t_mat4	rotate;
	t_mat4	tmp;

	q = quat_from_euler(obj.rx, obj.ry, obj.rz, cache);
	ft_scale_matrix(&scale, obj.sx, obj.sy, obj.sz);
	mat4_from_quat(&rotate, q);
	ft_translate_matrix(&translate, obj.tx, obj.ty, obj.tz);
	ft_matmul(&tmp, &rotate, &scale);
	ft_matmul(out, &translate, &tmp);
}

void apply_transform(t_point *p, t_mat4 *m)
{
	float	x;
	float	y;
	float	z;

	x = p->x;
	y = p->y;
	z = p->z;
	p->x = x * m->matrix[0] + y * m->matrix[1] + z * m->matrix[2] + m->matrix[3];
	p->y = x * m->matrix[4] + y * m->matrix[5] + z * m->matrix[6] + m->matrix[7];
	p->z = x * m->matrix[8] + y * m->matrix[9] + z * m->matrix[10] + m->matrix[11];
}

int main(void)
{
    t_trig_lookup cache;
    init_tables(&cache);
    printf("✔ Trig cache initialized.\n");

    float x_rad = M_PI / 4, y_rad = M_PI / 4, z_rad = M_PI / 4;
    t_quat q = quat_from_euler(x_rad, y_rad, z_rad, &cache);
    printf("✔ Quaternion: w=%.3f x=%.3f y=%.3f z=%.3f\n", q.w, q.x, q.y, q.z);

    t_mat4 rot;
    mat4_from_quat(&rot, q);
    printf("✔ Rotation matrix from quaternion:\n");
    for (int i = 0; i < MAT4_DIM; ++i) {
        if (i % 4 == 0) printf("\n");
        printf("%6.2f ", rot.matrix[i]);
    }

    t_mat4 scale;
    ft_scale_matrix(&scale, 2.0f, 1.0f, 0.5f);

    t_mat4 trans;
    ft_translate_matrix(&trans, 10.0f, 5.0f, -3.0f);

    t_mat4 temp;
    t_mat4 final;
    ft_matmul(&temp, &rot, &scale);
    ft_matmul(&final, &trans, &temp);
    printf("\n✔ Final transformation matrix:\n");
    for (int i = 0; i < MAT4_DIM; ++i) {
        if (i % 4 == 0) printf("\n");
        printf("%6.2f ", final.matrix[i]);
    }

    t_point p = {1.0f, 2.0f, 3.0f};
    apply_transform(&p, &final);
    printf("\n✔ Transformed point: x=%.2f y=%.2f z=%.2f\n", p.x, p.y, p.z);

    return 0;
}
