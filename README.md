CCHierarchiesSprite
===================

通过JSFL脚本将Flash动画数据导出，并采用层次节点的方式渲染的Cocos2d-iphone的一个扩展


功能
-------------------
   * 支持Avatar换装
   * 支持Retina
   * 支持Flash的经典插值动画
   * 支持Flash的引导层动画(通过JSFL进行转换为关键帧实现)
   * 支持Flash的Color Effect中的Advance(只支持颜色百分比)和Alpha模式
   * 支持MovieClip和Graphics的2D矩阵变换(缩放，旋转，切变，偏移，锚点)
   * 自动将MovieClip的原点每帧更新为CCNode的AnchorPoint坐标
   * 自动更新每帧的BoundingBox
   * 播放动画操作实现为Cocos2d方便的Action类
   * 通过缓存提高加载和动画计算的速度


局限
-------------------
   * 不支持Flash中MovieClip的嵌套
   * 不支持Flash的非经典插值动画
   * 不支持Flash的蒙版
   * 不支持Flash的滤镜
   * 不支持Flash的Shape变形动画
   * 不支持Flash的blend模式设置
   * 不支持Flash的元件循环旋转设置
   * 不支持Flash的骨骼动画


依赖
-------------------
   * cocos2d-iphone v2.0 以上
   * rapidxml v1.13 以上


性能
-------------------
   * iPad2同屏300个精灵在40FPS

   
