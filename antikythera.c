#include <pebble.h>
#define HOUR_RADIUS 10
#define HOUR_THICKNESS 3
#define MINUTE_RADIUS 16
#define MINUTE_THICKNESS 3
#define SECOND_RADIUS 23
#define SECOND_THICKNESS 3
#define HMS_RADIUS 27
#define SUN 0
#define MOON 1
#define VENUS 2
#define MARS 3
#define JUPITER 4
#define SUN_THICKNESS 10
#define SUN_STROKE GColorBlack
#define SUN_FILL GColorWhite
#define MOON_THICKNESS 10
#define MOON_STROKE GColorWhite
#define MOON_FILL GColorBlack
#define VENUS_SIZE 7
#define VENUS_RADIUS 32
#define MARS_SIZE 7
#define MARS_RADIUS 63
#define JUPITER_SIZE 7
#define JUPITER_RADIUS 75

static Window *window;
static Layer *timeLayer;
static Layer *astroLayer;
static Layer *rootLayer;

static int angle_45 = TRIG_MAX_ANGLE / 8;
static int angle_90 = TRIG_MAX_ANGLE / 4;
static int angle_180 = TRIG_MAX_ANGLE / 2;
static int angle_270 = 3 * TRIG_MAX_ANGLE / 4;

// Mars is just Venus rotated 180
static const GPathInfo VENUS_PATH_INFO = {
  .num_points = 3,
  .points = (GPoint[]) {{-VENUS_SIZE,-VENUS_SIZE*2},{VENUS_SIZE,-VENUS_SIZE*2},{0,0}}
};

// copied (and renamed) the DrawArc function from https://github.com/Jnmattern/Arc_2.0
// who, in turn, copied it from Cameron MacFarland (http://forums.getpebble.com/profile/12561/Cameron%20MacFarland)
static void graphics_draw_arc_cw(GContext *ctx, GPoint center, int radius, int thickness, int start_angle, int end_angle, GColor c) {
    int32_t xmin = 65535000, xmax = -65535000, ymin = 65535000, ymax = -65535000;
    int32_t cosStart, sinStart, cosEnd, sinEnd;
    int32_t r, t;

    while (start_angle < 0) start_angle += TRIG_MAX_ANGLE;
    while (end_angle < 0) end_angle += TRIG_MAX_ANGLE;

    start_angle %= TRIG_MAX_ANGLE;
    end_angle %= TRIG_MAX_ANGLE;

    if (end_angle == 0) end_angle = TRIG_MAX_ANGLE;

    if (start_angle > end_angle) {
        graphics_draw_arc_cw(ctx, center, radius, thickness, start_angle, TRIG_MAX_ANGLE, c);
        graphics_draw_arc_cw(ctx, center, radius, thickness, 0, end_angle, c);
    } else {
        // Calculate bounding box for the arc to be drawn
        cosStart = cos_lookup(start_angle);
        sinStart = sin_lookup(start_angle);
        cosEnd = cos_lookup(end_angle);
        sinEnd = sin_lookup(end_angle);

        r = radius;
        // Point 1: radius & start_angle
        t = r * cosStart;
        if (t < xmin) xmin = t;
        if (t > xmax) xmax = t;
        t = r * sinStart;
        if (t < ymin) ymin = t;
        if (t > ymax) ymax = t;

        // Point 2: radius & end_angle
        t = r * cosEnd;
        if (t < xmin) xmin = t;
        if (t > xmax) xmax = t;
        t = r * sinEnd;
        if (t < ymin) ymin = t;
        if (t > ymax) ymax = t;

        r = radius - thickness;
        // Point 3: radius-thickness & start_angle
        t = r * cosStart;
        if (t < xmin) xmin = t;
        if (t > xmax) xmax = t;
        t = r * sinStart;
        if (t < ymin) ymin = t;
        if (t > ymax) ymax = t;

        // Point 4: radius-thickness & end_angle
        t = r * cosEnd;
        if (t < xmin) xmin = t;
        if (t > xmax) xmax = t;
        t = r * sinEnd;
        if (t < ymin) ymin = t;
        if (t > ymax) ymax = t;

        // Normalization
        xmin /= TRIG_MAX_RATIO;
        xmax /= TRIG_MAX_RATIO;
        ymin /= TRIG_MAX_RATIO;
        ymax /= TRIG_MAX_RATIO;

        // Corrections if arc crosses X or Y axis
        if ((start_angle < angle_90) && (end_angle > angle_90)) {
            ymax = radius;
        }

        if ((start_angle < angle_180) && (end_angle > angle_180)) {
            xmin = -radius;
        }

        if ((start_angle < angle_270) && (end_angle > angle_270)) {
            ymin = -radius;
        }

        // Slopes for the two sides of the arc
        float sslope = (float)cosStart/ (float)sinStart;
        float eslope = (float)cosEnd / (float)sinEnd;

        if (end_angle == TRIG_MAX_ANGLE) eslope = -1000000;

        int ir2 = (radius - thickness) * (radius - thickness);
        int or2 = radius * radius;

        graphics_context_set_stroke_color(ctx, c);

        for (int x = xmin; x <= xmax; x++) {
            for (int y = ymin; y <= ymax; y++)
            {
                int x2 = x * x;
                int y2 = y * y;

                if (
                    (x2 + y2 < or2 && x2 + y2 >= ir2) && (
                        (y > 0 && start_angle < angle_180 && x <= y * sslope) ||
                        (y < 0 && start_angle > angle_180 && x >= y * sslope) ||
                        (y < 0 && start_angle <= angle_180) ||
                        (y == 0 && start_angle <= angle_180 && x < 0) ||
                        (y == 0 && start_angle == 0 && x > 0)
                    ) && (
                        (y > 0 && end_angle < angle_180 && x >= y * eslope) ||
                        (y < 0 && end_angle > angle_180 && x <= y * eslope) ||
                        (y > 0 && end_angle >= angle_180) ||
                        (y == 0 && end_angle >= angle_180 && x < 0) ||
                        (y == 0 && start_angle == 0 && x > 0)
                    )
                )
                graphics_draw_pixel(ctx, GPoint(center.x+x, center.y+y));
            }
        }
    }
}

// for completeness -- this isn't used.
static void graphics_draw_arc_ccw(GContext *ctx, GPoint center, int radius, int thickness, int start_angle, int end_angle, GColor c) {
  graphics_draw_arc_cw(ctx, center, radius, thickness, end_angle, start_angle, c);
}
 
static void bg_update(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void project_gpoint_to_edges(GPoint *point, Layer *layer, int angle) {
  GRect bounds = layer_get_bounds(layer);
  const GPoint center = grect_center_point(&bounds);
  angle %= TRIG_MAX_ANGLE;
  
  if (angle <= angle_45 || angle > angle_45 * 7) {
    point->x = center.x + bounds.size.w / 2;
    point->y = center.y + bounds.size.w * sin_lookup(angle) / cos_lookup(angle) / 2;
  } else if (angle > angle_45 * 3 && angle <= angle_45 * 5) {
    point->x = center.x - bounds.size.w / 2;
    point->y = center.y - bounds.size.w * sin_lookup(angle) / cos_lookup(angle) / 2;
  } else if (angle > angle_45 && angle <= angle_45 * 3) {
    point->x = center.x + bounds.size.h * cos_lookup(angle) / sin_lookup(angle) / 2;
    point->y = center.y + bounds.size.h / 2;
  } else if (angle > angle_45 * 5 && angle <= angle_45 * 7) {
    point->x = center.x - bounds.size.h * cos_lookup(angle) / sin_lookup(angle) / 2;
    point->y = center.y - bounds.size.h / 2;
  }
}

static void graphics_draw_ray(GContext *ctx, Layer *layer, GPoint center, int theta, int thickness_angle, GColor stroke_color, GColor fill_color) {
  GRect bounds = layer_get_bounds(layer);
  GPoint overshot = GPoint(0,0);
  GPoint undershot = GPoint(0,0);
  
  thickness_angle = thickness_angle * TRIG_MAX_ANGLE / 360;
  project_gpoint_to_edges(&undershot, layer, theta - thickness_angle);
  project_gpoint_to_edges(&overshot, layer, theta + thickness_angle);
  graphics_context_set_stroke_color(ctx, stroke_color);
  graphics_context_set_fill_color(ctx, fill_color);
  
  GPathInfo ray_info = {
    .num_points = 5,
    .points = (GPoint[]) {
      center,
      undershot,
      undershot,
      overshot,
      center
    }
  };
  // cornering =|
  if (undershot.x != overshot.x || undershot.y != overshot.y) {
    // ew
    if (undershot.x == bounds.size.w || overshot.x == bounds.size.w)
      ray_info.points[2].x = bounds.size.w;
    else
      ray_info.points[2].x = bounds.origin.x;
    // ew ew ew
    if (undershot.y == bounds.size.h || overshot.y == bounds.size.h)
      ray_info.points[2].y = bounds.size.h;
    else
      ray_info.points[2].y = bounds.origin.y;
  }
  GPath *ray = NULL;
  ray = gpath_create(&ray_info);
  gpath_draw_filled(ctx, ray);
  gpath_draw_outline(ctx, ray);
}

int get_body_angle(int body) {
  switch (body) {
    case SUN:
      return TRIG_MAX_ANGLE * 30 / 360;
      break;
    case MOON:
      return TRIG_MAX_ANGLE * 180 / 360;
      break;
    case VENUS:
      return TRIG_MAX_ANGLE * 270 / 360;
      break;
    case MARS:
      return TRIG_MAX_ANGLE * 270 / 360;
      break;
    case JUPITER:
      return TRIG_MAX_ANGLE * 270 / 360;
      break;
    default:
      return angle_90;
  }
}

static void draw_astro(Layer *layer, GContext *ctx) {
/*  time_t now = time(NULL);
  struct tm *t = localtime(&now);*/
  
  // draw dawn & dusk lines (background)
  int sunrise_angle = angle_90 * 0.2;
  int sunset_angle = angle_90 * 0.2;
  GRect bounds = layer_get_bounds(layer);
  const GPoint center = grect_center_point(&bounds);
  GPathInfo horizon_info = {
    .num_points = 5,
    .points = (GPoint []) {{bounds.origin.x, center.y - (bounds.size.w * sin_lookup(sunrise_angle) / (2 * cos_lookup(sunrise_angle)))},
                           {center.x, center.y},
                           {bounds.size.w, center.y - (bounds.size.w * sin_lookup(sunset_angle) / (2 * cos_lookup(sunset_angle)))},
                           {bounds.size.w, bounds.size.h},
                           {bounds.origin.x, bounds.size.h}
                          }
  };
  GPath *horizon = NULL;
  horizon = gpath_create(&horizon_info);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  gpath_draw_filled(ctx, horizon);
  gpath_draw_outline(ctx, horizon);
  
  // draw sun and moon rays
  int sun_angle = get_body_angle(SUN);
  int moon_angle = get_body_angle(MOON);
  int venus_angle = get_body_angle(VENUS);
  int mars_angle = get_body_angle(MARS);
  int jupiter_angle = get_body_angle(JUPITER);
  graphics_draw_ray(ctx, layer, center, sun_angle, SUN_THICKNESS, SUN_STROKE, SUN_FILL);
  graphics_draw_ray(ctx, layer, center, moon_angle, MOON_THICKNESS, MOON_STROKE, MOON_FILL);
  
  // draw planets
  GPath *venus = NULL;
  GPath *mars = NULL;
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  venus = gpath_create(&VENUS_PATH_INFO);
  gpath_move_to(venus, GPoint(center.x + VENUS_RADIUS * cos_lookup(venus_angle) / TRIG_MAX_RATIO,
                              center.y + VENUS_RADIUS * sin_lookup(venus_angle) / TRIG_MAX_RATIO));
  gpath_draw_filled(ctx, venus);
  gpath_draw_outline(ctx, venus);
  mars = gpath_create(&VENUS_PATH_INFO);
  gpath_move_to(mars, GPoint(center.x + MARS_RADIUS * cos_lookup(mars_angle) / TRIG_MAX_RATIO,
                              center.y + MARS_RADIUS * sin_lookup(mars_angle) / TRIG_MAX_RATIO));
  gpath_rotate_to(mars, TRIG_MAX_ANGLE / 2);
  gpath_draw_filled(ctx, mars);
  gpath_draw_outline(ctx, mars);
  
  GRect jupiter = GRect(center.x + JUPITER_RADIUS * cos_lookup(jupiter_angle) / TRIG_MAX_RATIO,
                        center.y + JUPITER_RADIUS * sin_lookup(jupiter_angle) / TRIG_MAX_RATIO,
                        JUPITER_SIZE * 2,
                        JUPITER_SIZE * 2);
  graphics_fill_rect(ctx, jupiter, 0, 0);
  graphics_draw_rect(ctx, jupiter);
}

static void draw_time(Layer *layer, GContext *ctx) {
    // draw the time in an analog clock at the center
    GRect bounds = layer_get_bounds(layer);
    const GPoint center = grect_center_point(&bounds);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int secondMarkers = t->tm_sec / 2;
    int i;
    
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, center, HMS_RADIUS);
    graphics_context_set_fill_color(ctx, GColorBlack);
    // draw the seconds
    if (secondMarkers % 30 != 0) {
      if (secondMarkers <= 15) {
        for (i=1; i<=secondMarkers; i++) {
          int angle = i * 4;
          GPoint marker = GPoint(center.x + SECOND_RADIUS * sin_lookup(angle * angle_180 / 30) / TRIG_MAX_RATIO, center.y - SECOND_RADIUS * cos_lookup(angle * angle_180 / 30) / TRIG_MAX_RATIO);
          graphics_fill_circle(ctx, marker, SECOND_THICKNESS);
        }
      } else {
        for (i=(secondMarkers % 15 + 1); i<=15; i++) {
          int angle = i * 4;
          GPoint marker = GPoint(center.x + SECOND_RADIUS * sin_lookup(angle * angle_180 / 30) / TRIG_MAX_RATIO, center.y - SECOND_RADIUS * cos_lookup(angle * angle_180 / 30) / TRIG_MAX_RATIO);
          graphics_fill_circle(ctx, marker, SECOND_THICKNESS);
        }
      }
    }
    // draw hour hand
    graphics_draw_arc_cw(ctx, center, HOUR_RADIUS, HOUR_THICKNESS, -angle_90, ((t->tm_hour * angle_180 / 6) - angle_90), GColorBlack);

    // draw minute hand
    graphics_draw_arc_cw(ctx, center, MINUTE_RADIUS, MINUTE_THICKNESS, -angle_90, ((t->tm_min * angle_180 / 30) - angle_90), GColorBlack);
}

static void update_astro(Layer *layer, GContext *ctx) {
    bg_update(layer, ctx);
    draw_astro(layer, ctx);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
    if (tick_time->tm_min % 5 == 0 && tick_time->tm_sec == 0) {
      layer_mark_dirty(astroLayer);
    }
    layer_mark_dirty(timeLayer);
}

static void init(void) {
    window = window_create();
    window_set_background_color(window, GColorWhite);
    window_stack_push(window, true);

    rootLayer = window_get_root_layer(window);
    astroLayer = layer_create(GRect(0, 0, 144, 168));
    timeLayer = layer_create(GRect(0, 0, 144, 168));
    layer_set_update_proc(timeLayer, draw_time);
    layer_set_update_proc(astroLayer, update_astro);
    layer_add_child(rootLayer, astroLayer);
    layer_add_child(rootLayer, timeLayer);

    tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
}

static void deinit(void) {
    tick_timer_service_unsubscribe();
    layer_destroy(timeLayer);
    layer_destroy(astroLayer);
    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}